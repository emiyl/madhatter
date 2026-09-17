#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "archive.h"
#include "layton_pack2.h"

static void expect_name(const char *label, const char *actual, const char *expected) {
    if (!actual || strcmp(actual, expected) != 0) {
        fprintf(stderr, "%s: expected '%s' got '%s'\n", label, expected, actual ? actual : "(null)");
        __builtin_trap();
    }
}

int main(void) {
    mh_archive src = {0};
    mh_archive out = {0};
    mh_buffer blob = {0};
    const uint8_t a_data[] = {'A', 'B', 'C'};
    const uint8_t b_data[] = {'H', 'i', '!', '\n'};
    const char *a_name = "a.bin";
    const char *b_name = "b.txt";

    if (mh_archive_init(&src) != 0) {
        fprintf(stderr, "src archive init failed\n");
        return 1;
    }

    if (mh_archive_add_file(&src, a_name, a_data, sizeof(a_data)) != 0 ||
        mh_archive_add_file(&src, b_name, b_data, sizeof(b_data)) != 0) {
        fprintf(stderr, "archive add failed\n");
        mh_archive_free(&src);
        return 1;
    }

    if (mh_archive_save_layton_pack2(&src, &blob) != 0) {
        fprintf(stderr, "pack2 save failed\n");
        mh_archive_free(&src);
        return 1;
    }

    if (mh_archive_load_layton_pack2(&out, blob.data, blob.len) != 0) {
        fprintf(stderr, "pack2 load failed\n");
        mh_buffer_free(&blob);
        mh_archive_free(&src);
        return 1;
    }

    if (out.count != 2u) {
        fprintf(stderr, "pack2 count mismatch: expected 2 got %zu\n", out.count);
        mh_archive_free(&out);
        mh_buffer_free(&blob);
        mh_archive_free(&src);
        return 1;
    }

    expect_name("pack2 entry 0", out.entries[0].name, a_name);
    expect_name("pack2 entry 1", out.entries[1].name, b_name);

    if (out.entries[0].asset.len != sizeof(a_data) || memcmp(out.entries[0].asset.data, a_data, sizeof(a_data)) != 0) {
        fprintf(stderr, "pack2 payload 0 mismatch\n");
        mh_archive_free(&out);
        mh_buffer_free(&blob);
        mh_archive_free(&src);
        return 1;
    }

    if (out.entries[1].asset.len != sizeof(b_data) || memcmp(out.entries[1].asset.data, b_data, sizeof(b_data)) != 0) {
        fprintf(stderr, "pack2 payload 1 mismatch\n");
        mh_archive_free(&out);
        mh_buffer_free(&blob);
        mh_archive_free(&src);
        return 1;
    }

    mh_archive_free(&out);
    mh_buffer_free(&blob);
    mh_archive_free(&src);
    puts("layton pack2 roundtrip passed");
    return 0;
}
