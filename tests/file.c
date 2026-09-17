#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "file.h"
#include "huffman.h"

static void expect_bytes(const char *label, const uint8_t *actual, size_t actual_len,
                         const uint8_t *expected, size_t expected_len) {
    if (actual_len != expected_len || memcmp(actual, expected, expected_len) != 0) {
        fprintf(stderr, "%s: mismatch\n", label);
        fprintf(stderr, "expected len=%zu, actual len=%zu\n", expected_len, actual_len);
        fprintf(stderr, "expected bytes: ");
        for (size_t i = 0; i < expected_len; ++i) {
            fprintf(stderr, "%02x ", expected[i]);
        }
        fprintf(stderr, "\nactual bytes: ");
        for (size_t i = 0; i < actual_len; ++i) {
            fprintf(stderr, "%02x ", actual[i]);
        }
        fprintf(stderr, "\n");
        __builtin_trap();
    }
}

int main(void) {
    const uint8_t lz10_input[] = {0x10, 0x03, 0x00, 0x00, 0x00, 'A', 'B', 'C'};
    const uint8_t lz10_expected[] = {'A', 'B', 'C'};
    const uint8_t rle_input[] = {0x30, 0x04, 0x00, 0x00, 0x80u, 0xABu, 0x00u, 0x12u};
    const uint8_t rle_expected[] = {0xAB, 0xAB, 0xAB, 0x12};
    const uint8_t huffman_input[] = {0x41, 0x41, 0x42, 0x43, 0x43, 0x43, 0x44};
    mh_buffer lz10_out = {0};
    mh_buffer rle_out = {0};
    mh_buffer huffman_out = {0};
    mh_buffer huffman_compressed = {0};

    if (mh_file_decompress_detected(lz10_input, sizeof(lz10_input), &lz10_out) != 0) {
        fprintf(stderr, "file detect lz10 failed\n");
        return 1;
    }
    expect_bytes("file detect lz10", lz10_out.data, lz10_out.len, lz10_expected, sizeof(lz10_expected));
    mh_buffer_free(&lz10_out);

    if (mh_file_decompress_detected(rle_input, sizeof(rle_input), &rle_out) != 0) {
        fprintf(stderr, "file detect rle failed\n");
        return 1;
    }
    expect_bytes("file detect rle", rle_out.data, rle_out.len, rle_expected, sizeof(rle_expected));
    mh_buffer_free(&rle_out);

    if (mh_huffman_compress(huffman_input, sizeof(huffman_input), &huffman_compressed, 0) != 0) {
        fprintf(stderr, "huffman compress for file wrapper failed\n");
        return 1;
    }
    if (mh_file_decompress_detected(huffman_compressed.data, huffman_compressed.len, &huffman_out) != 0) {
        fprintf(stderr, "file detect huffman failed\n");
        mh_buffer_free(&huffman_compressed);
        return 1;
    }
    expect_bytes("file detect huffman", huffman_out.data, huffman_out.len, huffman_input, sizeof(huffman_input));

    mh_buffer_free(&huffman_compressed);
    mh_buffer_free(&huffman_out);
    puts("file wrapper test passed");
    return 0;
}
