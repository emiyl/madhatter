#include "rle.h"

#include <stdlib.h>

static int append_repeated_bytes(mh_buffer *out, uint8_t value, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) {
        if (mh_buffer_append(out, &value, 1) != 0) {
            return -1;
        }
    }
    return 0;
}

int mh_rle_decompress(const uint8_t *src, size_t src_len, mh_buffer *out) {
    mh_reader reader;
    const uint8_t *payload = src;
    size_t payload_len = src_len;

    if (!src || !out) {
        return -1;
    }

    if (src_len >= 4u && src[0] == 0x30u) {
        payload = src + 4u;
        payload_len = src_len - 4u;
    }

    mh_buffer_init(out);
    mh_reader_init(&reader, payload, payload_len);

    while (mh_reader_has_data(&reader)) {
        uint8_t flag = mh_reader_read_u8(&reader);
        int is_compressed = (flag & 0x80u) != 0u;
        size_t length;
        uint8_t value;

        if (is_compressed) {
            length = (size_t)(flag & 0x7Fu) + 3u;
            if (!mh_reader_has_data(&reader)) {
                return -1;
            }
            value = mh_reader_read_u8(&reader);
            if (append_repeated_bytes(out, value, length) != 0) {
                return -1;
            }
        } else {
            length = (size_t)(flag & 0x7Fu) + 1u;
            if (reader.pos + length > reader.len) {
                return -1;
            }
            for (size_t i = 0; i < length; ++i) {
                if (mh_buffer_append(out, &reader.data[reader.pos], 1) != 0) {
                    return -1;
                }
                reader.pos += 1;
            }
        }
    }

    return 0;
}

int mh_rle_compress(const uint8_t *src, size_t src_len, mh_buffer *out) {
    mh_writer writer;
    size_t i = 0;

    if (!src || !out) {
        return -1;
    }

    mh_buffer_init(out);
    mh_writer_init(&writer);

    while (i < src_len) {
        uint8_t value = src[i];
        size_t run_len = 1;

        while (i + run_len < src_len && src[i + run_len] == value && run_len < 130u) {
            run_len += 1;
        }

        if (run_len >= 3) {
            uint8_t flag = (uint8_t)(0x80u | (run_len - 3u));
            if (mh_writer_write_u8(&writer, flag) != 0 || mh_writer_write_u8(&writer, value) != 0) {
                mh_writer_free(&writer);
                return -1;
            }
            i += run_len;
            continue;
        }

        {
            size_t literal_len = 1;
            while (i + literal_len < src_len && literal_len < 128u) {
                size_t j = i + literal_len;
                if (src[j] == value && literal_len + 1 >= 3) {
                    break;
                }
                if (j + 1 < src_len && src[j] == src[j + 1] && literal_len + 2 >= 3) {
                    break;
                }
                literal_len += 1;
            }

            if (literal_len > 1 || (i + literal_len < src_len && src[i + literal_len] == value)) {
                uint8_t flag = (uint8_t)(literal_len - 1u);
                if (mh_writer_write_u8(&writer, flag) != 0) {
                    mh_writer_free(&writer);
                    return -1;
                }
                for (size_t j = 0; j < literal_len; ++j) {
                    if (mh_writer_write_u8(&writer, src[i + j]) != 0) {
                        mh_writer_free(&writer);
                        return -1;
                    }
                }
                i += literal_len;
                continue;
            }

            if (mh_writer_write_u8(&writer, 0u) != 0 || mh_writer_write_u8(&writer, src[i]) != 0) {
                mh_writer_free(&writer);
                return -1;
            }
            i += 1;
        }
    }

    out->data = writer.data;
    out->len = writer.len;
    return 0;
}
