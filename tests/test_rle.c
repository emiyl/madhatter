#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mh_rle.h"

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
    const uint8_t input[] = {0xAB, 0xAB, 0xAB, 0x12};
    const uint8_t expected[] = {0xAB, 0xAB, 0xAB, 0x12};
    const uint8_t compressed[] = {0x80u, 0xABu, 0x00u, 0x12u};
    mh_buffer out = {0};
    mh_buffer roundtrip = {0};

    int rc = mh_rle_decompress(compressed, sizeof(compressed), &out);
    if (rc != 0) {
        fprintf(stderr, "mh_rle_decompress failed rc=%d\n", rc);
        return 1;
    }

    expect_bytes("rle decode", out.data, out.len, expected, sizeof(expected));
    mh_buffer_free(&out);

    rc = mh_rle_compress(input, sizeof(input), &roundtrip);
    if (rc != 0) {
        fprintf(stderr, "mh_rle_compress failed rc=%d\n", rc);
        return 1;
    }

    if (roundtrip.len != sizeof(compressed) || memcmp(roundtrip.data, compressed, sizeof(compressed)) != 0) {
        fprintf(stderr, "rle compress mismatch\n");
        mh_buffer_free(&roundtrip);
        return 1;
    }

    mh_buffer_free(&roundtrip);
    puts("rle test passed");
    return 0;
}
