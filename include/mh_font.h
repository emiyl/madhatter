#ifndef MH_FONT_H
#define MH_FONT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* One glyph, pre-expanded to its full advance cell (NFTR HDWC "length"),
 * with the raw tile bitmap blitted at HDWC "start". Pixel value is glyph
 * intensity (0 or 255); render_string treats it as alpha over white. */
typedef struct {
    uint32_t codepoint;
    uint8_t *bitmap; /* cell_width * cell_height, owned */
    int cell_width;
    int cell_height;
    int advance; /* cell_width, plus +1 char-bias unless disable_bias */
    int disable_bias;
} mh_font_glyph;

/* C port of widebrim's NftrTiles (Nintendo font resource: PLGC/HDWC/PAMC blocks). */
typedef struct {
    mh_font_glyph *glyphs;
    size_t glyph_count;
    int tile_width;
    int tile_height;
} mh_font;

int mh_font_load_nftr(mh_font *font, const uint8_t *data, size_t len);
void mh_font_free(mh_font *font);

const mh_font_glyph *mh_font_find_glyph(const mh_font *font, uint32_t codepoint);

/* Renders a UTF-8 string as a single line to a newly malloc'd RGBA8888 buffer
 * (white glyphs, glyph intensity used as alpha). Caller frees *out_pixels. */
int mh_font_render_string(const mh_font *font,
                          const char *utf8_text,
                          uint8_t **out_pixels,
                          int *out_width,
                          int *out_height);

#ifdef __cplusplus
}
#endif

#endif
