#include "mh_font.h"

#include <stdlib.h>
#include <string.h>

#include "stream.h"

#define MH_FONT_MAX_GLYPHS 8192u
#define MH_FONT_CHAR_BIAS 1

static void mh_reader_skip(mh_reader *r, size_t n) {
    mh_reader_seek(r, r->pos + n);
}

static uint32_t mh_cp1252_high_to_unicode(uint8_t b) {
    static const uint16_t table[32] = {
        0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
        0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
        0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
        0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178
    };
    return table[b - 0x80u];
}

static uint32_t mh_decode_char16(uint8_t byte_a, uint8_t byte_b, uint8_t decode_mode) {
    if (decode_mode == 1) {
        return ((uint32_t)byte_a << 8) | byte_b;
    }

    if (byte_a != 0 && byte_b != 0) {
        return 0xFFFFFFFFu;
    }
    {
        uint8_t value = byte_a != 0 ? byte_a : byte_b;
        if (decode_mode == 3 && value >= 0x80u) {
            return mh_cp1252_high_to_unicode(value);
        }
        return (uint32_t)value;
    }
}

static void mh_font_glyph_expand(mh_font_glyph *glyph, int tile_w, int tile_h, int start, int width, int length) {
    uint8_t *cell;
    int x, y;

    (void)width;
    if (length <= 0) {
        return;
    }

    cell = (uint8_t *)calloc((size_t)length * (size_t)tile_h, 1);
    if (!cell) {
        return;
    }

    for (y = 0; y < tile_h; ++y) {
        for (x = 0; x < tile_w; ++x) {
            int dst_x = x + start;
            if (dst_x >= 0 && dst_x < length) {
                cell[y * length + dst_x] = glyph->bitmap[y * tile_w + x];
            }
        }
    }

    free(glyph->bitmap);
    glyph->bitmap = cell;
    glyph->cell_width = length;
    glyph->cell_height = tile_h;
    glyph->advance = length;
    if (width != 0 && start != 0 && width != start) {
        glyph->disable_bias = 1;
    }
}

int mh_font_load_nftr(mh_font *font, const uint8_t *data, size_t len) {
    mh_reader reader;
    uint32_t length_header32;
    uint16_t length_header, count_blocks;
    uint32_t length_font_info;
    uint8_t font_info_decode_mode;
    uint32_t offset_plgc, offset_hdwc, offset_pamc;
    uint32_t length_plgc;
    int tile_w, tile_h;
    uint16_t length_tile;
    uint32_t count_tiles;
    uint32_t count_pixels;
    uint32_t i;

    if (!font || !data) {
        return -1;
    }
    memset(font, 0, sizeof(*font));
    (void)length_header32;
    (void)length_font_info;

    mh_reader_init(&reader, data, len);

    mh_reader_skip(&reader, 8u);
    mh_reader_read_u32_le(&reader); /* filesize, unused */
    length_header = mh_reader_read_u16_le(&reader);
    count_blocks = mh_reader_read_u16_le(&reader);
    (void)count_blocks;

    mh_reader_seek(&reader, (size_t)length_header + 4u);
    length_font_info = mh_reader_read_u32_le(&reader);
    mh_reader_skip(&reader, 1u);
    mh_reader_read_u8(&reader); // fontInfoHeight, unused
    mh_reader_skip(&reader, 3u);
    mh_reader_read_u8(&reader); // fontInfoWidth, unused
    mh_reader_skip(&reader, 1u);
    font_info_decode_mode = mh_reader_read_u8(&reader);
    offset_plgc = mh_reader_read_u32_le(&reader) - 8u;
    offset_hdwc = mh_reader_read_u32_le(&reader) - 8u;
    offset_pamc = mh_reader_read_u32_le(&reader) - 8u;

    mh_reader_seek(&reader, (size_t)offset_plgc + 4u);
    length_plgc = mh_reader_read_u32_le(&reader);
    tile_w = mh_reader_read_u8(&reader);
    tile_h = mh_reader_read_u8(&reader);
    length_tile = mh_reader_read_u16_le(&reader);
    mh_reader_skip(&reader, 2u);
    mh_reader_read_u8(&reader); // depth, unused
    mh_reader_read_u8(&reader); // rotate, unused

    if (tile_w <= 0 || tile_h <= 0 || length_tile == 0u || length_plgc < 16u) {
        return -1;
    }
    count_tiles = (length_plgc - 16u) / length_tile;
    if (count_tiles == 0u || count_tiles > MH_FONT_MAX_GLYPHS) {
        return -1;
    }
    count_pixels = (uint32_t)tile_w * (uint32_t)tile_h;

    font->glyphs = (mh_font_glyph *)calloc(count_tiles, sizeof(mh_font_glyph));
    if (!font->glyphs) {
        return -1;
    }
    font->glyph_count = count_tiles;
    font->tile_width = tile_w;
    font->tile_height = tile_h;

    for (i = 0; i < count_tiles; ++i) {
        mh_font_glyph *glyph = &font->glyphs[i];
        uint32_t pixel_index = 0u;
        uint16_t b;

        glyph->bitmap = (uint8_t *)calloc((size_t)tile_w * (size_t)tile_h, 1);
        glyph->cell_width = tile_w;
        glyph->cell_height = tile_h;
        glyph->advance = tile_w;
        glyph->codepoint = 0xFFFFFFFFu;
        if (!glyph->bitmap) {
            return -1;
        }

        for (b = 0; b < length_tile; ++b) {
            uint8_t byte = mh_reader_read_u8(&reader);
            int bit;
            for (bit = 0; bit < 8; ++bit) {
                int enabled = (byte & (1u << (7 - bit))) != 0;
                if (pixel_index < count_pixels) {
                    glyph->bitmap[pixel_index] = enabled ? 255u : 0u;
                }
                ++pixel_index;
            }
        }
    }

    mh_reader_seek(&reader, (size_t)offset_hdwc + 8u);
    {
        uint16_t char_first = mh_reader_read_u16_le(&reader);
        uint16_t char_last = mh_reader_read_u16_le(&reader);
        uint32_t g;
        mh_reader_skip(&reader, 4u);
        for (g = char_first; g <= (uint32_t)char_last && g < font->glyph_count; ++g) {
            int8_t start = (int8_t)mh_reader_read_u8(&reader);
            uint8_t width = mh_reader_read_u8(&reader);
            uint8_t length = mh_reader_read_u8(&reader);
            mh_font_glyph_expand(&font->glyphs[g], tile_w, tile_h, start, width, length);
        }
    }

    {
        uint32_t offset_next = offset_pamc;
        uint32_t guard = 0u;
        while (offset_next != 0u && guard++ < 64u) {
            uint16_t code_first, code_last;
            uint32_t type_pamc;
            uint32_t code;

            mh_reader_seek(&reader, (size_t)offset_next + 4u);
            mh_reader_read_u32_le(&reader); // lengthPamc, unused
            code_first = mh_reader_read_u16_le(&reader);
            code_last = mh_reader_read_u16_le(&reader);
            type_pamc = mh_reader_read_u32_le(&reader);
            offset_next = mh_reader_read_u32_le(&reader);
            if (offset_next != 0u) {
                offset_next -= 8u;
            }

            if (type_pamc == 0u) {
                uint32_t glyph_index = mh_reader_read_u16_le(&reader);
                for (code = code_first; code <= (uint32_t)code_last; ++code, ++glyph_index) {
                    if (glyph_index < font->glyph_count) {
                        font->glyphs[glyph_index].codepoint =
                            mh_decode_char16((uint8_t)(code >> 8), (uint8_t)code, font_info_decode_mode);
                    }
                }
            } else if (type_pamc == 1u) {
                for (code = code_first; code <= (uint32_t)code_last; ++code) {
                    int16_t glyph_index = (int16_t)mh_reader_read_u16_le(&reader);
                    if (glyph_index >= 0 && (uint32_t)glyph_index < font->glyph_count) {
                        font->glyphs[glyph_index].codepoint =
                            mh_decode_char16((uint8_t)(code >> 8), (uint8_t)code, font_info_decode_mode);
                    }
                }
            } else if (type_pamc == 2u) {
                uint16_t count_defs = mh_reader_read_u16_le(&reader);
                uint16_t d;
                for (d = 0; d < count_defs; ++d) {
                    uint8_t hi = mh_reader_read_u8(&reader);
                    uint8_t lo = mh_reader_read_u8(&reader);
                    uint32_t glyph_index = mh_reader_read_u16_le(&reader);
                    if (glyph_index < font->glyph_count) {
                        font->glyphs[glyph_index].codepoint = mh_decode_char16(hi, lo, font_info_decode_mode);
                    }
                }
            }
        }
    }

    return 0;
}

void mh_font_free(mh_font *font) {
    size_t i;
    if (!font) {
        return;
    }
    for (i = 0; i < font->glyph_count; ++i) {
        free(font->glyphs[i].bitmap);
    }
    free(font->glyphs);
    font->glyphs = NULL;
    font->glyph_count = 0;
}

const mh_font_glyph *mh_font_find_glyph(const mh_font *font, uint32_t codepoint) {
    size_t i;
    if (!font) {
        return NULL;
    }
    for (i = 0; i < font->glyph_count; ++i) {
        if (font->glyphs[i].codepoint == codepoint) {
            return &font->glyphs[i];
        }
    }
    return NULL;
}

static uint32_t mh_utf8_next(const char **p) {
    const unsigned char *s = (const unsigned char *)*p;
    uint32_t cp;
    int extra;

    if (s[0] < 0x80u) {
        cp = s[0];
        extra = 0;
    } else if ((s[0] & 0xE0u) == 0xC0u) {
        cp = s[0] & 0x1Fu;
        extra = 1;
    } else if ((s[0] & 0xF0u) == 0xE0u) {
        cp = s[0] & 0x0Fu;
        extra = 2;
    } else if ((s[0] & 0xF8u) == 0xF0u) {
        cp = s[0] & 0x07u;
        extra = 3;
    } else {
        *p += 1;
        return 0xFFFDu;
    }

    *p += 1;
    s += 1;
    while (extra-- > 0 && (s[0] & 0xC0u) == 0x80u) {
        cp = (cp << 6) | (s[0] & 0x3Fu);
        *p += 1;
        s += 1;
    }
    return cp;
}

int mh_font_render_string(const mh_font *font,
                          const char *utf8_text,
                          uint8_t **out_pixels,
                          int *out_width,
                          int *out_height) {
    const mh_font_glyph **matched;
    size_t count = 0, capacity = 32;
    const char *p = utf8_text;
    int total_width = 0, max_height = 0;
    uint8_t *pixels;
    size_t i;
    int x_offset;

    if (!font || !utf8_text || !out_pixels || !out_width || !out_height) {
        return -1;
    }
    *out_pixels = NULL;
    *out_width = 0;
    *out_height = 0;

    matched = (const mh_font_glyph **)malloc(capacity * sizeof(*matched));
    if (!matched) {
        return -1;
    }

    while (*p != '\0') {
        uint32_t cp = mh_utf8_next(&p);
        const mh_font_glyph *glyph = mh_font_find_glyph(font, cp);
        if (!glyph) {
            continue;
        }
        if (count == capacity) {
            capacity *= 2;
            matched = (const mh_font_glyph **)realloc(matched, capacity * sizeof(*matched));
            if (!matched) {
                return -1;
            }
        }
        matched[count++] = glyph;
        total_width += glyph->advance + (glyph->disable_bias ? 0 : MH_FONT_CHAR_BIAS);
        if (glyph->cell_height > max_height) {
            max_height = glyph->cell_height;
        }
    }

    if (count == 0 || total_width <= 0 || max_height <= 0) {
        free(matched);
        return -1;
    }

    pixels = (uint8_t *)calloc((size_t)total_width * (size_t)max_height, 4u);
    if (!pixels) {
        free(matched);
        return -1;
    }

    x_offset = 0;
    for (i = 0; i < count; ++i) {
        const mh_font_glyph *glyph = matched[i];
        int gx, gy;
        for (gy = 0; gy < glyph->cell_height; ++gy) {
            for (gx = 0; gx < glyph->cell_width; ++gx) {
                uint8_t intensity = glyph->bitmap[gy * glyph->cell_width + gx];
                size_t dst = ((size_t)gy * (size_t)total_width + (size_t)(x_offset + gx)) * 4u;
                pixels[dst + 0u] = 0u;
                pixels[dst + 1u] = 0u;
                pixels[dst + 2u] = 0u;
                pixels[dst + 3u] = intensity;
            }
        }
        x_offset += glyph->advance + (glyph->disable_bias ? 0 : MH_FONT_CHAR_BIAS);
    }

    free(matched);
    *out_pixels = pixels;
    *out_width = total_width;
    *out_height = max_height;
    return 0;
}
