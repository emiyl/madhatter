#include "lz10.h"

#include <stdlib.h>

static uint32_t mh_read_u24_le(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16);
}

int mh_lz10_decompress(const uint8_t *src, size_t src_len, mh_buffer *out) {
    size_t src_pos = 0;
    size_t out_pos = 0;
    size_t out_cap = 0;
    uint8_t *out_data = NULL;

    if (!src || !out) {
        return -1;
    }

    mh_buffer_init(out);

    if (src_len < 4) {
        return -1;
    }

    if (src[0] != 0x10) {
        return -1;
    }

    out_cap = mh_read_u24_le(&src[1]);
    out_data = (uint8_t *)calloc(out_cap ? out_cap : 1, sizeof(uint8_t));
    if (!out_data) {
        return -1;
    }

    src_pos = 4;
    while (src_pos < src_len && out_pos < out_cap) {
        uint8_t control = src[src_pos++];

        for (int bit = 7; bit >= 0 && src_pos < src_len && out_pos < out_cap; --bit) {
            if ((control & (1u << bit)) != 0u) {
                uint8_t b1;
                uint8_t b2;
                uint16_t disp;
                uint16_t length;
                size_t offset;

                if (src_pos + 1 >= src_len) {
                    free(out_data);
                    return -1;
                }

                b1 = src[src_pos++];
                b2 = src[src_pos++];
                disp = (uint16_t)((((uint16_t)(b1 & 0x0Fu) << 8) | b2) + 1u);
                length = (uint16_t)(((uint16_t)(b1 >> 4) & 0x0Fu) + 3u);

                if (disp == 0u || disp > out_pos) {
                    free(out_data);
                    return -1;
                }

                offset = out_pos - disp;
                for (uint16_t i = 0; i < length && out_pos < out_cap; ++i) {
                    out_data[out_pos++] = out_data[offset++];
                    if (offset >= out_pos) {
                        offset = out_pos - disp;
                    }
                }
            } else {
                if (src_pos >= src_len) {
                    free(out_data);
                    return -1;
                }
                out_data[out_pos++] = src[src_pos++];
            }
        }
    }

    out->data = out_data;
    out->len = out_cap;
    return 0;
}

int mh_lz10_compress(const uint8_t *src, size_t src_len, mh_buffer *out) {
    (void)src;
    (void)src_len;
    (void)out;
    return -1;
}
