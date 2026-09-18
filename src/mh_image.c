#include "mh_image.h"

#include <stdlib.h>
#include <string.h>

#include "stream.h"

#define MH_IMAGE_TILE_PIXELS 64u /* 8x8 */
#define MH_IMAGE_MAX_PALETTE 256u
#define MH_IMAGE_MAX_TILES 65536u
#define MH_IMAGE_MAX_DIM 4096

static int mh_image_read_bytes(mh_reader *r, uint8_t *dst, size_t n) {
    if (r->pos + n > r->len) {
        return -1;
    }
    memcpy(dst, r->data + r->pos, n);
    r->pos += n;
    return 0;
}

static void mh_image_bgr555_to_rgb888(uint16_t packed, uint8_t *r, uint8_t *g, uint8_t *b) {
    unsigned rr = packed & 0x1Fu;
    unsigned gg = (packed >> 5) & 0x1Fu;
    unsigned bb = (packed >> 10) & 0x1Fu;
    *r = (uint8_t)((rr * 255u + 15u) / 31u);
    *g = (uint8_t)((gg * 255u + 15u) / 31u);
    *b = (uint8_t)((bb * 255u + 15u) / 31u);
}

int mh_image_decode_static_arc(const uint8_t *data,
                                size_t len,
                                uint8_t **out_pixels,
                                int *out_width,
                                int *out_height) {
    mh_reader reader;
    uint32_t length_palette;
    uint8_t *palette_rgb = NULL;
    uint32_t tile_count;
    uint8_t *tiles = NULL;
    uint16_t tiles_w, tiles_h;
    int width, height;
    uint32_t tile_map_count;
    uint16_t *tile_map = NULL;
    uint8_t *pixels = NULL;
    uint32_t i;
    int status = -1;

    if (!data || !out_pixels || !out_width || !out_height) {
        return -1;
    }
    *out_pixels = NULL;
    *out_width = 0;
    *out_height = 0;

    mh_reader_init(&reader, data, len);

    length_palette = mh_reader_read_u32_le(&reader);
    if (length_palette == 0u || length_palette > MH_IMAGE_MAX_PALETTE) {
        return -1;
    }

    palette_rgb = (uint8_t *)malloc((size_t)length_palette * 3u);
    if (!palette_rgb) {
        return -1;
    }
    for (i = 0; i < length_palette; ++i) {
        uint16_t packed = mh_reader_read_u16_le(&reader);
        mh_image_bgr555_to_rgb888(packed, &palette_rgb[i * 3u], &palette_rgb[i * 3u + 1u], &palette_rgb[i * 3u + 2u]);
    }

    tile_count = mh_reader_read_u32_le(&reader);
    if (tile_count == 0u || tile_count > MH_IMAGE_MAX_TILES) {
        goto fail;
    }

    tiles = (uint8_t *)malloc((size_t)tile_count * MH_IMAGE_TILE_PIXELS);
    if (!tiles) {
        goto fail;
    }
    for (i = 0; i < tile_count; ++i) {
        uint8_t raw[MH_IMAGE_TILE_PIXELS];
        uint32_t p;
        if (mh_image_read_bytes(&reader, raw, MH_IMAGE_TILE_PIXELS) != 0) {
            goto fail;
        }
        for (p = 0; p < MH_IMAGE_TILE_PIXELS; ++p) {
            tiles[(size_t)i * MH_IMAGE_TILE_PIXELS + p] = (uint8_t)(raw[p] % length_palette);
        }
    }

    tiles_w = mh_reader_read_u16_le(&reader);
    tiles_h = mh_reader_read_u16_le(&reader);
    width = (int)tiles_w * 8;
    height = (int)tiles_h * 8;
    if (tiles_w == 0u || tiles_h == 0u || width > MH_IMAGE_MAX_DIM || height > MH_IMAGE_MAX_DIM) {
        goto fail;
    }

    tile_map_count = (uint32_t)tiles_w * (uint32_t)tiles_h;
    tile_map = (uint16_t *)malloc((size_t)tile_map_count * sizeof(uint16_t));
    if (!tile_map) {
        goto fail;
    }
    for (i = 0; i < tile_map_count; ++i) {
        tile_map[i] = mh_reader_read_u16_le(&reader);
    }
    if (reader.pos > reader.len) {
        goto fail;
    }

    pixels = (uint8_t *)malloc((size_t)width * (size_t)height * 4u);
    if (!pixels) {
        goto fail;
    }
    
    for (i = 0; i < (uint32_t)width * (uint32_t)height; ++i) {
        pixels[i * 4u + 0u] = palette_rgb[0];
        pixels[i * 4u + 1u] = palette_rgb[1];
        pixels[i * 4u + 2u] = palette_rgb[2];
        pixels[i * 4u + 3u] = 0u;
    }

    for (i = 0; i < tile_map_count; ++i) {
        uint16_t selected = tile_map[i];
        uint16_t tile_index = selected & 0x03FFu;
        int flip_x = (selected & 0x0800u) != 0;
        int flip_y = (selected & 0x0400u) != 0;
        uint32_t tile_x = i % tiles_w;
        uint32_t tile_y = i / tiles_w;
        int px, py;

        if (tile_index >= 0x03FFu || tile_count == 0u) {
            continue;
        }

        {
            uint32_t actual_tile = tile_index % tile_count;
            const uint8_t *tile_data = &tiles[(size_t)actual_tile * MH_IMAGE_TILE_PIXELS];

            for (py = 0; py < 8; ++py) {
                for (px = 0; px < 8; ++px) {
                    int src_x = flip_x ? (7 - px) : px;
                    int src_y = flip_y ? (7 - py) : py;
                    uint8_t index = tile_data[src_y * 8 + src_x];
                    int dst_x = (int)tile_x * 8 + px;
                    int dst_y = (int)tile_y * 8 + py;
                    size_t dst = ((size_t)dst_y * (size_t)width + (size_t)dst_x) * 4u;

                    pixels[dst + 0u] = palette_rgb[(size_t)index * 3u + 0u];
                    pixels[dst + 1u] = palette_rgb[(size_t)index * 3u + 1u];
                    pixels[dst + 2u] = palette_rgb[(size_t)index * 3u + 2u];
                    pixels[dst + 3u] = index == 0u ? 0u : 255u;
                }
            }
        }
    }

    *out_pixels = pixels;
    *out_width = width;
    *out_height = height;
    pixels = NULL;
    status = 0;

fail:
    free(palette_rgb);
    free(tiles);
    free(tile_map);
    free(pixels);
    return status;
}
