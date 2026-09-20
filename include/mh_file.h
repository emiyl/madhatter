#ifndef MH_FILE_H
#define MH_FILE_H

#include <stddef.h>
#include <stdint.h>

#include "mh_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    MH_COMP_NONE = 0,
    MH_COMP_LZ10 = 0x10,
    MH_COMP_HUFFMAN_4 = 0x24,
    MH_COMP_HUFFMAN_8 = 0x28,
    MH_COMP_RLE = 0x30
};

typedef struct {
    uint8_t *data;
    size_t len;
} mh_file;

void mh_file_init(mh_file *file);
void mh_file_free(mh_file *file);
int mh_file_set_data(mh_file *file, const uint8_t *data, size_t len);
int mh_file_detect_compression_type(const uint8_t *src, size_t src_len, size_t offset);
int mh_file_decompress(const uint8_t *src, size_t src_len, mh_buffer *out, size_t offset);
int mh_file_decompress_detected(const uint8_t *src, size_t src_len, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
