#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
    const uint8_t input[] = {0x41, 0x41, 0x42, 0x43, 0x43, 0x43, 0x44};
    const uint8_t expected[] = {0x41, 0x41, 0x42, 0x43, 0x43, 0x43, 0x44};
    const uint8_t expected_8bit[] = {0x28, 0x07, 0x00, 0x00, 0x03, 0x80, 0x43, 0x80, 0x41, 0xc0, 0x42, 0x44, 0x00, 0x00, 0x38, 0xac, 0x00, 0x00, 0x00, 0x00};
    mh_buffer compressed = {0};
    mh_buffer out = {0};

    if (mh_huffman_compress(input, sizeof(input), &compressed, 0) != 0) {
        fprintf(stderr, "mh_huffman_compress failed\n");
        return 1;
    }

    expect_bytes("huffman exact 8-bit format", compressed.data, compressed.len,
                 expected_8bit, sizeof(expected_8bit));

    if (mh_huffman_decompress(compressed.data, compressed.len, &out, 0) != 0) {
        fprintf(stderr, "mh_huffman_decompress failed\n");
        mh_buffer_free(&compressed);
        return 1;
    }

    expect_bytes("huffman roundtrip", out.data, out.len, expected, sizeof(expected));

    mh_buffer_free(&compressed);
    mh_buffer_free(&out);
    puts("huffman test passed");
    return 0;
}
