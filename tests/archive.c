#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archive.h"

int main(void) {
    mh_archive archive = {0};
    mh_buffer extracted = {0};
    const uint8_t uncompressed[] = {'A', 'B', 'C'};
    const uint8_t lz10_payload[] = {
        0x10, 0x03, 0x00, 0x00,
        0x00,
        'A', 'B', 'C'
    };
    mh_archive_entry *entry = NULL;

    if (mh_archive_init(&archive) != 0) {
        fprintf(stderr, "archive init failed\n");
        return 1;
    }

    if (mh_archive_add_file(&archive, "hello.bin", uncompressed, sizeof(uncompressed)) != 0) {
        fprintf(stderr, "archive add uncompressed file failed\n");
        mh_archive_free(&archive);
        return 1;
    }

    if (mh_archive_add_file(&archive, "hello.lz10", lz10_payload, sizeof(lz10_payload)) != 0) {
        fprintf(stderr, "archive add compressed file failed\n");
        mh_archive_free(&archive);
        return 1;
    }

    if (archive.count != 2u) {
        fprintf(stderr, "archive count mismatch: expected 2 got %zu\n", archive.count);
        mh_archive_free(&archive);
        return 1;
    }

    entry = mh_archive_get(&archive, "hello.bin");
    if (!entry || entry->asset.len != sizeof(uncompressed) || memcmp(entry->asset.data, uncompressed, sizeof(uncompressed)) != 0) {
        fprintf(stderr, "archive lookup for hello.bin failed\n");
        mh_archive_free(&archive);
        return 1;
    }

    entry = mh_archive_get(&archive, "hello.lz10");
    if (!entry) {
        fprintf(stderr, "archive lookup for hello.lz10 failed\n");
        mh_archive_free(&archive);
        return 1;
    }

    if (mh_asset_decompress(&entry->asset, &extracted) != 0) {
        fprintf(stderr, "archive asset decompression failed\n");
        mh_archive_free(&archive);
        return 1;
    }

    if (extracted.len != 3u || memcmp(extracted.data, "ABC", 3) != 0) {
        fprintf(stderr, "archive asset decompression mismatch\n");
        mh_buffer_free(&extracted);
        mh_archive_free(&archive);
        return 1;
    }

    mh_buffer_free(&extracted);
    mh_archive_free(&archive);
    puts("archive wrapper test passed");
    return 0;
}
