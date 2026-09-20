#ifndef MH_FONT_H
#define MH_FONT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t codepoint;
    uint8_t *bitmap;
    int cell_width;
    int cell_height;
    int advance;
    int disable_bias;
} mh_font_glyph;

typedef struct {
    mh_font_glyph *glyphs;
    size_t glyph_count;
    int tile_width;
    int tile_height;
} mh_font;

int mh_font_load_nftr(mh_font *font, const uint8_t *data, size_t len);
void mh_font_free(mh_font *font);

const mh_font_glyph *mh_font_find_glyph(const mh_font *font, uint32_t codepoint);

int mh_font_render_string(const mh_font *font,
                          const char *utf8_text,
                          uint8_t **out_pixels,
                          int *out_width,
                          int *out_height);

#ifdef __cplusplus
}
#endif

#endif
