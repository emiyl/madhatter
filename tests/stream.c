#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "stream.h"

int main(void) {
    const uint8_t bytes[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    mh_reader reader;
    mh_writer writer;
    uint8_t out[8] = {0};

    mh_reader_init(&reader, bytes, sizeof(bytes));
    if (mh_reader_read_u8(&reader) != 0x01) {
        fprintf(stderr, "reader u8 failed\n");
        return 1;
    }
    if (mh_reader_read_u16_le(&reader) != 0x0302) {
        fprintf(stderr, "reader u16 failed\n");
        return 1;
    }
    if (mh_reader_read_u32_le(&reader) != 0x07060504u) {
        fprintf(stderr, "reader u32 failed\n");
        return 1;
    }

    mh_writer_init(&writer);
    mh_writer_write_u16_le(&writer, 0x1234);
    mh_writer_write_u32_le(&writer, 0x01020304u);
    mh_writer_write_bytes(&writer, (const uint8_t *)"OK", 2);

    if (writer.len != 2 + 4 + 2) {
        fprintf(stderr, "writer length mismatch\n");
        return 1;
    }

    memcpy(out, writer.data, writer.len);
    if (out[0] != 0x34 || out[1] != 0x12 || out[2] != 0x04 || out[3] != 0x03 || out[4] != 0x02 || out[5] != 0x01 || out[6] != 'O' || out[7] != 'K') {
        fprintf(stderr, "writer bytes mismatch\n");
        return 1;
    }

    mh_writer_free(&writer);
    puts("stream test passed");
    return 0;
}
