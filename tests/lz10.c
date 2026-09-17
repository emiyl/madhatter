#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lz10.h"

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
    const uint8_t compressed[] = {
        0x10, 0x03, 0x00, 0x00,
        0x00,
        'A', 'B', 'C'
    };

    const uint8_t expected[] = {'A', 'B', 'C'};

    mh_buffer out = {0};
    int rc = mh_lz10_decompress(compressed, sizeof(compressed), &out);
    if (rc != 0) {
        fprintf(stderr, "mh_lz10_decompress failed rc=%d\n", rc);
        return 1;
    }

    expect_bytes("lz10 decode", out.data, out.len, expected, sizeof(expected));
    mh_buffer_free(&out);

    puts("lz10 test passed");
    return 0;
}
