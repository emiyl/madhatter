#ifndef MH_RLE_H
#define MH_RLE_H

#include <stddef.h>
#include <stdint.h>

#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

int mh_rle_decompress(const uint8_t *src, size_t src_len, mh_buffer *out);
int mh_rle_compress(const uint8_t *src, size_t src_len, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
