#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "asset.h"

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
    const uint8_t expected[] = {'A', 'B', 'C'};
    mh_asset asset = {0};
    mh_buffer out = {0};

    if (mh_asset_init_from_bytes(&asset, lz10_input, sizeof(lz10_input)) != 0) {
        fprintf(stderr, "mh_asset_init_from_bytes failed\n");
        return 1;
    }

    if (mh_asset_decompress(&asset, &out) != 0) {
        fprintf(stderr, "mh_asset_decompress failed\n");
        mh_asset_free(&asset);
        return 1;
    }

    expect_bytes("asset lz10 decode", out.data, out.len, expected, sizeof(expected));

    mh_buffer_free(&out);
    mh_asset_free(&asset);
    puts("asset wrapper test passed");
    return 0;
}
