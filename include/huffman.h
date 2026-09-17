#ifndef MH_HUFFMAN_H
#define MH_HUFFMAN_H

#include <stddef.h>
#include <stdint.h>

#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

int mh_huffman_compress(const uint8_t *src, size_t src_len, mh_buffer *out, int half_byte_blocks);
int mh_huffman_decompress(const uint8_t *src, size_t src_len, mh_buffer *out, int half_byte_blocks);

#ifdef __cplusplus
}
#endif

#endif
