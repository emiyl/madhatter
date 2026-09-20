#include "mh_file.h"

#include "mh_huffman.h"
#include "mh_lz10.h"
#include "mh_rle.h"

#include <stdlib.h>
#include <string.h>

void mh_file_init(mh_file *file) {
    if (!file) {
        return;
    }
    file->data = NULL;
    file->len = 0u;
}

void mh_file_free(mh_file *file) {
    if (!file) {
        return;
    }
    free(file->data);
    file->data = NULL;
    file->len = 0u;
}

int mh_file_set_data(mh_file *file, const uint8_t *data, size_t len) {
    uint8_t *copy = NULL;
    if (!file) {
        return -1;
    }
    free(file->data);
    file->data = NULL;
    file->len = 0u;
    if (!data || len == 0u) {
        return 0;
    }
    copy = (uint8_t *)malloc(len);
    if (!copy) {
        return -1;
    }
    memcpy(copy, data, len);
    file->data = copy;
    file->len = len;
    return 0;
}

int mh_file_detect_compression_type(const uint8_t *src, size_t src_len, size_t offset) {
    if (!src || src_len < offset + 1u) {
        return MH_COMP_NONE;
    }

    switch (src[offset]) {
        case MH_COMP_LZ10:
            if (src_len < offset + 4u) {
                return MH_COMP_NONE;
            }
            if ((src[offset + 1] | src[offset + 2] | src[offset + 3]) == 0u) {
                return MH_COMP_NONE;
            }
            return MH_COMP_LZ10;
        case MH_COMP_HUFFMAN_4:
            return MH_COMP_HUFFMAN_4;
        case MH_COMP_HUFFMAN_8:
            return MH_COMP_HUFFMAN_8;
        case MH_COMP_RLE:
            return MH_COMP_RLE;
        default:
            return MH_COMP_NONE;
    }
}

int mh_file_decompress(const uint8_t *src, size_t src_len, mh_buffer *out, size_t offset) {
    int compression_type;

    if (!src || !out || src_len < offset + 1u) {
        return -1;
    }

    compression_type = mh_file_detect_compression_type(src, src_len, offset);
    if (compression_type == MH_COMP_NONE) {
        return -1;
    }

    switch (compression_type) {
        case MH_COMP_LZ10:
            return mh_lz10_decompress(src + offset, src_len - offset, out);
        case MH_COMP_HUFFMAN_4:
            return mh_huffman_decompress(src + offset, src_len - offset, out, 1);
        case MH_COMP_HUFFMAN_8:
            return mh_huffman_decompress(src + offset, src_len - offset, out, 0);
        case MH_COMP_RLE:
            return mh_rle_decompress(src + offset, src_len - offset, out);
        default:
            return -1;
    }
}

int mh_file_decompress_detected(const uint8_t *src, size_t src_len, mh_buffer *out) {
    size_t offset = 0u;
    int type = MH_COMP_NONE;

    if (!src || !out) {
        return -1;
    }

    type = mh_file_detect_compression_type(src, src_len, offset);
    if (type == MH_COMP_NONE && src_len >= 8u) {
        offset = 4u;
        type = mh_file_detect_compression_type(src, src_len, offset);
    }
    if (type == MH_COMP_NONE) {
        return -1;
    }

    return mh_file_decompress(src, src_len, out, offset);
}
