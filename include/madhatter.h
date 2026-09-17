#ifndef MADHATTER_H
#define MADHATTER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *data;
    size_t len;
} mh_buffer;

void mh_buffer_init(mh_buffer *buf);
void mh_buffer_free(mh_buffer *buf);
int mh_buffer_resize(mh_buffer *buf, size_t new_len);
int mh_buffer_append(mh_buffer *buf, const uint8_t *src, size_t src_len);

int mh_lz10_decompress(const uint8_t *src, size_t src_len, mh_buffer *out);
int mh_lz10_compress(const uint8_t *src, size_t src_len, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
