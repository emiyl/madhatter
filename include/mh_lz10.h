#ifndef MH_LZ10_H
#define MH_LZ10_H

#include <stddef.h>
#include <stdint.h>

#include "mh_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

int mh_lz10_decompress(const uint8_t *src, size_t src_len, mh_buffer *out);
int mh_lz10_compress(const uint8_t *src, size_t src_len, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
