#include "madhatter.h"

#include <stdlib.h>
#include <string.h>

void mh_buffer_init(mh_buffer *buf) {
    if (!buf) {
        return;
    }
    buf->data = NULL;
    buf->len = 0;
}

void mh_buffer_free(mh_buffer *buf) {
    if (!buf) {
        return;
    }
    free(buf->data);
    buf->data = NULL;
    buf->len = 0;
}

int mh_buffer_resize(mh_buffer *buf, size_t new_len) {
    uint8_t *tmp;
    if (!buf) {
        return -1;
    }
    if (new_len == 0) {
        free(buf->data);
        buf->data = NULL;
        buf->len = 0;
        return 0;
    }
    tmp = (uint8_t *)realloc(buf->data, new_len);
    if (!tmp) {
        return -1;
    }
    buf->data = tmp;
    buf->len = new_len;
    return 0;
}

int mh_buffer_append(mh_buffer *buf, const uint8_t *src, size_t src_len) {
    size_t new_len;
    uint8_t *tmp;
    if (!buf || !src) {
        return -1;
    }
    new_len = buf->len + src_len;
    tmp = (uint8_t *)realloc(buf->data, new_len);
    if (!tmp) {
        return -1;
    }
    memcpy(tmp + buf->len, src, src_len);
    buf->data = tmp;
    buf->len = new_len;
    return 0;
}

static uint32_t read_u24_le(const uint8_t *p) {
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

    out_cap = read_u24_le(&src[1]);
    out_data = (uint8_t *)calloc(out_cap ? out_cap : 1, sizeof(uint8_t));
    if (!out_data) {
        return -1;
    }

    src_pos = 4;
    while (src_pos < src_len && out_pos < out_cap) {
        uint8_t control = src[src_pos++];

        for (int bit = 0; bit < 8 && src_pos < src_len && out_pos < out_cap; ++bit) {
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
                disp = (uint16_t)(((uint16_t)(b1 & 0x0Fu) << 8) | b2);
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
