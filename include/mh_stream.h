#ifndef MH_STREAM_H
#define MH_STREAM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *data;
    size_t len;
} mh_buffer;

typedef struct {
    const uint8_t *data;
    size_t len;
    size_t pos;
} mh_reader;

typedef struct {
    uint8_t *data;
    size_t len;
    size_t cap;
} mh_writer;

void mh_buffer_init(mh_buffer *buf);
void mh_buffer_free(mh_buffer *buf);
int mh_buffer_resize(mh_buffer *buf, size_t new_len);
int mh_buffer_append(mh_buffer *buf, const uint8_t *src, size_t src_len);

void mh_reader_init(mh_reader *reader, const uint8_t *src, size_t len);
uint8_t mh_reader_read_u8(mh_reader *reader);
uint16_t mh_reader_read_u16_le(mh_reader *reader);
uint32_t mh_reader_read_u32_le(mh_reader *reader);
void mh_reader_seek(mh_reader *reader, size_t pos);
size_t mh_reader_tell(const mh_reader *reader);
int mh_reader_has_data(const mh_reader *reader);

void mh_writer_init(mh_writer *writer);
void mh_writer_free(mh_writer *writer);
int mh_writer_write_bytes(mh_writer *writer, const uint8_t *src, size_t len);
int mh_writer_write_u16_le(mh_writer *writer, uint16_t value);
int mh_writer_write_u32_le(mh_writer *writer, uint32_t value);
int mh_writer_write_u8(mh_writer *writer, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif
