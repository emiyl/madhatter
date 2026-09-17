#include "stream.h"

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

void mh_reader_init(mh_reader *reader, const uint8_t *src, size_t len) {
    if (!reader) {
        return;
    }
    reader->data = src;
    reader->len = len;
    reader->pos = 0;
}

uint8_t mh_reader_read_u8(mh_reader *reader) {
    if (!reader || !mh_reader_has_data(reader)) {
        return 0;
    }
    return reader->data[reader->pos++];
}

uint16_t mh_reader_read_u16_le(mh_reader *reader) {
    uint16_t value = 0;
    if (!reader || reader->pos + 2 > reader->len) {
        return 0;
    }
    value = (uint16_t)reader->data[reader->pos];
    value |= (uint16_t)reader->data[reader->pos + 1] << 8;
    reader->pos += 2;
    return value;
}

uint32_t mh_reader_read_u32_le(mh_reader *reader) {
    uint32_t value = 0;
    if (!reader || reader->pos + 4 > reader->len) {
        return 0;
    }
    value = (uint32_t)reader->data[reader->pos];
    value |= (uint32_t)reader->data[reader->pos + 1] << 8;
    value |= (uint32_t)reader->data[reader->pos + 2] << 16;
    value |= (uint32_t)reader->data[reader->pos + 3] << 24;
    reader->pos += 4;
    return value;
}

void mh_reader_seek(mh_reader *reader, size_t pos) {
    if (!reader) {
        return;
    }
    if (pos > reader->len) {
        reader->pos = reader->len;
        return;
    }
    reader->pos = pos;
}

size_t mh_reader_tell(const mh_reader *reader) {
    if (!reader) {
        return 0;
    }
    return reader->pos;
}

int mh_reader_has_data(const mh_reader *reader) {
    if (!reader) {
        return 0;
    }
    return reader->pos < reader->len;
}

void mh_writer_init(mh_writer *writer) {
    if (!writer) {
        return;
    }
    writer->data = NULL;
    writer->len = 0;
    writer->cap = 0;
}

void mh_writer_free(mh_writer *writer) {
    if (!writer) {
        return;
    }
    free(writer->data);
    writer->data = NULL;
    writer->len = 0;
    writer->cap = 0;
}

int mh_writer_write_bytes(mh_writer *writer, const uint8_t *src, size_t len) {
    uint8_t *tmp;
    size_t new_cap;
    if (!writer || !src) {
        return -1;
    }
    if (writer->len + len > writer->cap) {
        new_cap = writer->cap == 0 ? 16 : writer->cap;
        while (writer->len + len > new_cap) {
            if (new_cap < 4096) {
                new_cap *= 2;
            } else {
                new_cap += 4096;
            }
        }
        tmp = (uint8_t *)realloc(writer->data, new_cap);
        if (!tmp) {
            return -1;
        }
        writer->data = tmp;
        writer->cap = new_cap;
    }
    memcpy(writer->data + writer->len, src, len);
    writer->len += len;
    return 0;
}

int mh_writer_write_u8(mh_writer *writer, uint8_t value) {
    return mh_writer_write_bytes(writer, &value, 1);
}

int mh_writer_write_u16_le(mh_writer *writer, uint16_t value) {
    uint8_t bytes[2];
    bytes[0] = (uint8_t)(value & 0xFFu);
    bytes[1] = (uint8_t)((value >> 8) & 0xFFu);
    return mh_writer_write_bytes(writer, bytes, sizeof(bytes));
}

int mh_writer_write_u32_le(mh_writer *writer, uint32_t value) {
    uint8_t bytes[4];
    bytes[0] = (uint8_t)(value & 0xFFu);
    bytes[1] = (uint8_t)((value >> 8) & 0xFFu);
    bytes[2] = (uint8_t)((value >> 16) & 0xFFu);
    bytes[3] = (uint8_t)((value >> 24) & 0xFFu);
    return mh_writer_write_bytes(writer, bytes, sizeof(bytes));
}
