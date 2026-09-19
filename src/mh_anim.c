#include "mh_anim.h"

#include <stdlib.h>
#include <string.h>

#include "stream.h"

#define MH_ANIM_MAX_SUBIMAGES 256u
#define MH_ANIM_MAX_TILES 4096u
#define MH_ANIM_MAX_DIM 1024
#define MH_ANIM_MAX_ANIMS 256u

typedef struct {
    uint32_t subimage_index;
    int offset_x, offset_y;
    int width, height;
    uint8_t *raw;
} mh_anim_raw_tile;

static int mh_anim_read_bytes(mh_reader *r, uint8_t *dst, size_t n) {
    if (r->pos + n > r->len) {
        return -1;
    }
    memcpy(dst, r->data + r->pos, n);
    r->pos += n;
    return 0;
}

static void mh_anim_bgr555_to_rgb888(uint16_t packed, uint8_t *r, uint8_t *g, uint8_t *b) {
    unsigned rr = packed & 0x1Fu;
    unsigned gg = (packed >> 5) & 0x1Fu;
    unsigned bb = (packed >> 10) & 0x1Fu;
    *r = (uint8_t)((rr * 255u + 15u) / 31u);
    *g = (uint8_t)((gg * 255u + 15u) / 31u);
    *b = (uint8_t)((bb * 255u + 15u) / 31u);
}

/* Unpacks `raw` (packed at `bpp` bits/pixel, LSB-first sub-pixel order) into
 * one palette-index byte per pixel, clamped modulo palette_count. */
static void mh_anim_unpack_tile(const uint8_t *raw, int width, int height, int bpp, uint32_t palette_count,
                                 uint8_t *out_indices) {
    int x = 0, y = 0;
    size_t byte_count = (size_t)width * (size_t)height * (size_t)bpp / 8u;
    size_t i;
    uint32_t mask = (1u << bpp) - 1u;

    for (i = 0; i < byte_count; ++i) {
        uint8_t packed = raw[i];
        int sub;
        for (sub = 0; sub < 8 / bpp; ++sub) {
            uint8_t index = (uint8_t)((packed & mask) % (palette_count ? palette_count : 1u));
            out_indices[y * width + x] = index;
            packed = (uint8_t)(packed >> bpp);
            ++x;
            if (x == width) {
                x = 0;
                ++y;
            }
        }
    }
}

int mh_anim_decode_arc(const uint8_t *data, size_t len, mh_anim_image *out) {
    mh_reader reader;
    uint16_t count_subimage;
    uint16_t bpp_exp;
    int bpp;
    int (*sub_res)[2] = NULL; /* [w,h] per subimage */
    mh_anim_raw_tile *tiles = NULL;
    size_t tile_count = 0, tile_capacity = 0;
    uint32_t count_colours;
    uint8_t *palette_rgb = NULL;
    uint32_t i;
    int status = -1;

    if (!data || !out) {
        return -1;
    }
    memset(out, 0, sizeof(*out));

    mh_reader_init(&reader, data, len);

    count_subimage = mh_reader_read_u16_le(&reader);
    bpp_exp = mh_reader_read_u16_le(&reader);
    if (count_subimage == 0u || count_subimage > MH_ANIM_MAX_SUBIMAGES || bpp_exp == 0u || bpp_exp > 4u) {
        return -1;
    }
    bpp = 1 << (bpp_exp - 1u);

    sub_res = (int(*)[2])malloc((size_t)count_subimage * sizeof(*sub_res));
    if (!sub_res) {
        return -1;
    }

    for (i = 0; i < count_subimage; ++i) {
        uint32_t tiles_here, t;
        sub_res[i][0] = mh_reader_read_u16_le(&reader);
        sub_res[i][1] = mh_reader_read_u16_le(&reader);
        tiles_here = mh_reader_read_u32_le(&reader);
        if (tiles_here > MH_ANIM_MAX_TILES) {
            goto fail;
        }
        for (t = 0; t < tiles_here; ++t) {
            mh_anim_raw_tile tile;
            uint16_t exp_w, exp_h;
            size_t raw_len;

            memset(&tile, 0, sizeof(tile));
            tile.subimage_index = i;
            tile.offset_x = mh_reader_read_u16_le(&reader);
            tile.offset_y = mh_reader_read_u16_le(&reader);
            exp_w = mh_reader_read_u16_le(&reader);
            exp_h = mh_reader_read_u16_le(&reader);
            tile.width = 1 << (3 + exp_w);
            tile.height = 1 << (3 + exp_h);
            if (tile.width <= 0 || tile.height <= 0 || tile.width > MH_ANIM_MAX_DIM || tile.height > MH_ANIM_MAX_DIM) {
                goto fail;
            }

            raw_len = (size_t)tile.width * (size_t)tile.height * (size_t)bpp / 8u;
            tile.raw = (uint8_t *)malloc(raw_len ? raw_len : 1u);
            if (!tile.raw || mh_anim_read_bytes(&reader, tile.raw, raw_len) != 0) {
                free(tile.raw);
                goto fail;
            }

            if (tile_count == tile_capacity) {
                size_t new_cap = tile_capacity == 0 ? 16 : tile_capacity * 2;
                mh_anim_raw_tile *grown = (mh_anim_raw_tile *)realloc(tiles, new_cap * sizeof(*tiles));
                if (!grown) {
                    free(tile.raw);
                    goto fail;
                }
                tiles = grown;
                tile_capacity = new_cap;
            }
            tiles[tile_count++] = tile;
        }
    }

    count_colours = mh_reader_read_u32_le(&reader);
    if (count_colours == 0u || count_colours > 256u) {
        goto fail;
    }
    palette_rgb = (uint8_t *)malloc((size_t)count_colours * 3u);
    if (!palette_rgb) {
        goto fail;
    }
    for (i = 0; i < count_colours; ++i) {
        uint16_t packed = mh_reader_read_u16_le(&reader);
        mh_anim_bgr555_to_rgb888(packed, &palette_rgb[i * 3u], &palette_rgb[i * 3u + 1u], &palette_rgb[i * 3u + 2u]);
    }

    /* Decode each tile's packed pixels now that the palette size is known. */
    for (i = 0; i < tile_count; ++i) {
        mh_anim_raw_tile *tile = &tiles[i];
        uint8_t *indices = (uint8_t *)malloc((size_t)tile->width * (size_t)tile->height);
        if (!indices) {
            goto fail;
        }
        mh_anim_unpack_tile(tile->raw, tile->width, tile->height, bpp, count_colours, indices);
        free(tile->raw);
        tile->raw = indices; /* now holds palette indices, one byte per pixel */
    }

    out->frames = (mh_anim_frame *)calloc(count_subimage, sizeof(mh_anim_frame));
    if (!out->frames) {
        goto fail;
    }
    out->frame_count = count_subimage;

    for (i = 0; i < count_subimage; ++i) {
        int width = sub_res[i][0];
        int height = sub_res[i][1];
        uint8_t *pixels;
        size_t p;

        if (width <= 0 || height <= 0 || width > MH_ANIM_MAX_DIM || height > MH_ANIM_MAX_DIM) {
            goto fail;
        }
        pixels = (uint8_t *)calloc((size_t)width * (size_t)height, 4u);
        if (!pixels) {
            goto fail;
        }
        out->frames[i].pixels = pixels;
        out->frames[i].width = width;
        out->frames[i].height = height;

        for (p = 0; p < tile_count; ++p) {
            const mh_anim_raw_tile *tile = &tiles[p];
            int tx, ty;
            if (tile->subimage_index != i) {
                continue;
            }
            for (ty = 0; ty < tile->height; ++ty) {
                int dst_y = tile->offset_y + ty;
                if (dst_y < 0 || dst_y >= height) {
                    continue;
                }
                for (tx = 0; tx < tile->width; ++tx) {
                    int dst_x = tile->offset_x + tx;
                    uint8_t index;
                    size_t dst;
                    if (dst_x < 0 || dst_x >= width) {
                        continue;
                    }
                    index = tile->raw[ty * tile->width + tx];
                    dst = ((size_t)dst_y * (size_t)width + (size_t)dst_x) * 4u;
                    pixels[dst + 0u] = palette_rgb[(size_t)index * 3u + 0u];
                    pixels[dst + 1u] = palette_rgb[(size_t)index * 3u + 1u];
                    pixels[dst + 2u] = palette_rgb[(size_t)index * 3u + 2u];
                    pixels[dst + 3u] = index == 0u ? 0u : 255u;
                }
            }
        }
    }

    mh_reader_seek(&reader, reader.pos + 30u);
    {
        uint32_t count_anims = mh_reader_read_u32_le(&reader);
        uint32_t a;
        if (count_anims > MH_ANIM_MAX_ANIMS) {
            goto fail;
        }
        out->animations = (mh_anim_animation *)calloc(count_anims ? count_anims : 1u, sizeof(mh_anim_animation));
        if (!out->animations) {
            goto fail;
        }
        out->animation_count = count_anims;

        for (a = 0; a < count_anims; ++a) {
            uint8_t name_buf[31];
            size_t n;
            if (mh_anim_read_bytes(&reader, name_buf, 30u) != 0) {
                goto fail;
            }
            name_buf[30] = '\0';
            for (n = 0; n < 30u; ++n) {
                if (name_buf[n] == '\0') {
                    break;
                }
            }
            memcpy(out->animations[a].name, name_buf, n);
            out->animations[a].name[n] = '\0';
            out->animations[a].first_frame_index = -1;
        }

        for (a = 0; a < count_anims; ++a) {
            uint32_t count_frames = mh_reader_read_u32_le(&reader);
            uint32_t *index_keyframe = NULL;
            uint32_t *duration_frame = NULL;
            uint32_t *index_frame = NULL;
            uint32_t k;
            uint32_t best_key = UINT32_MAX;
            int best_frame = -1;

            if (count_frames == 0u || count_frames > 1024u) {
                out->animations[a].first_frame_index = -1;
                out->animations[a].keyframe_count = 0u;
                out->animations[a].keyframes = NULL;
                continue;
            }

            index_keyframe = (uint32_t *)malloc((size_t)count_frames * sizeof(uint32_t));
            duration_frame = (uint32_t *)malloc((size_t)count_frames * sizeof(uint32_t));
            index_frame = (uint32_t *)malloc((size_t)count_frames * sizeof(uint32_t));
            if (!index_keyframe || !duration_frame || !index_frame) {
                free(index_keyframe);
                free(duration_frame);
                free(index_frame);
                goto fail;
            }
            for (k = 0; k < count_frames; ++k) {
                index_keyframe[k] = mh_reader_read_u32_le(&reader);
            }
            for (k = 0; k < count_frames; ++k) {
                duration_frame[k] = mh_reader_read_u32_le(&reader);
            }
            for (k = 0; k < count_frames; ++k) {
                index_frame[k] = mh_reader_read_u32_le(&reader);
            }

            out->animations[a].keyframe_count = count_frames;
            out->animations[a].keyframes = (mh_anim_keyframe *)calloc(count_frames, sizeof(mh_anim_keyframe));
            if (!out->animations[a].keyframes) {
                free(index_keyframe);
                free(duration_frame);
                free(index_frame);
                goto fail;
            }

            for (k = 0; k < count_frames; ++k) {
                out->animations[a].keyframes[k].keyframe_index = index_keyframe[k];
                out->animations[a].keyframes[k].duration_frames = duration_frame[k];
                out->animations[a].keyframes[k].frame_index = (int)index_frame[k];

                if (index_keyframe[k] < best_key) {
                    best_key = index_keyframe[k];
                    best_frame = (int)index_frame[k];
                }
            }

            out->animations[a].first_frame_index = best_frame;
            free(index_keyframe);
            free(duration_frame);
            free(index_frame);
        }
    }

    status = 0;

fail:
    if (status != 0) {
        mh_anim_free(out);
    }
    free(sub_res);
    free(palette_rgb);
    for (i = 0; i < tile_count; ++i) {
        free(tiles[i].raw);
    }
    free(tiles);
    return status;
}

void mh_anim_free(mh_anim_image *image) {
    size_t i;
    if (!image) {
        return;
    }
    for (i = 0; i < image->frame_count; ++i) {
        free(image->frames[i].pixels);
    }
    free(image->frames);
    free(image->animations);
    memset(image, 0, sizeof(*image));
}

const mh_anim_animation *mh_anim_get_animation_by_name(const mh_anim_image *image, const char *name) {
    size_t i;
    if (!image || !name) {
        return NULL;
    }
    for (i = 0; i < image->animation_count; ++i) {
        if (strcmp(image->animations[i].name, name) == 0) {
            return &image->animations[i];
        }
    }
    return NULL;
}

const mh_anim_frame *mh_anim_get_frame_by_animation_name(const mh_anim_image *image, const char *name) {
    const mh_anim_animation *animation = mh_anim_get_animation_by_name(image, name);
    if (!animation) {
        return NULL;
    }
    if (animation->first_frame_index >= 0 && (size_t)animation->first_frame_index < image->frame_count) {
        return &image->frames[animation->first_frame_index];
    }
    return NULL;
}

int mh_anim_get_animation_frame_index(const mh_anim_image *image, const char *name, uint64_t elapsed_ms) {
    const mh_anim_animation *animation = mh_anim_get_animation_by_name(image, name);
    size_t i;
    uint64_t total_duration_ms = 0u;
    uint64_t remaining = elapsed_ms;

    if (!image || !name || !animation || animation->keyframe_count == 0u) {
        return (animation && animation->first_frame_index >= 0) ? animation->first_frame_index : -1;
    }

    for (i = 0; i < animation->keyframe_count; ++i) {
        uint64_t duration_ms = (animation->keyframes[i].duration_frames == 0u)
                                   ? 1u
                                   : (((uint64_t)animation->keyframes[i].duration_frames * 1000u) / 60u);
        total_duration_ms += duration_ms;
    }
    if (total_duration_ms == 0u) {
        return animation->first_frame_index;
    }
    remaining %= total_duration_ms;

    for (i = 0; i < animation->keyframe_count; ++i) {
        uint64_t duration_ms = (animation->keyframes[i].duration_frames == 0u)
                                   ? 1u
                                   : (((uint64_t)animation->keyframes[i].duration_frames * 1000u) / 60u);
        if (remaining < duration_ms) {
            return animation->keyframes[i].frame_index;
        }
        remaining -= duration_ms;
    }
    return animation->keyframes[animation->keyframe_count - 1u].frame_index;
}
