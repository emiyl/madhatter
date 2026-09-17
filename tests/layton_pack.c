#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "layton_pack.h"

int main(void) {
    mh_archive archive = {0};
    mh_archive loaded = {0};
    mh_buffer blob = {0};
    mh_archive_entry *entry = NULL;
    const uint8_t a_data[] = {'A', 'B', 'C'};
    const uint8_t b_data[] = {'D', 'E'};

    if (mh_archive_init(&archive) != 0) {
        fprintf(stderr, "archive init failed\n");
        return 1;
    }

    if (mh_archive_add_file(&archive, "a.bin", a_data, sizeof(a_data)) != 0 ||
        mh_archive_add_file(&archive, "b.bin", b_data, sizeof(b_data)) != 0) {
        fprintf(stderr, "archive add failed\n");
        mh_archive_free(&archive);
        return 1;
    }

    if (mh_archive_save_layton_pack(&archive, &blob, 1) != 0) {
        fprintf(stderr, "layton pack save failed\n");
        mh_archive_free(&archive);
        return 1;
    }

    if (mh_archive_load_layton_pack(&loaded, blob.data, blob.len, 1) != 0) {
        fprintf(stderr, "layton pack load failed\n");
        mh_buffer_free(&blob);
        mh_archive_free(&archive);
        return 1;
    }

    if (loaded.count != 2u) {
        fprintf(stderr, "layton pack count mismatch: %zu\n", loaded.count);
        mh_archive_free(&loaded);
        mh_buffer_free(&blob);
        mh_archive_free(&archive);
        return 1;
    }

    entry = mh_archive_get(&loaded, "a.bin");
    if (!entry || entry->asset.len != sizeof(a_data) || memcmp(entry->asset.data, a_data, sizeof(a_data)) != 0) {
        fprintf(stderr, "a.bin mismatch\n");
        mh_archive_free(&loaded);
        mh_buffer_free(&blob);
        mh_archive_free(&archive);
        return 1;
    }

    entry = mh_archive_get(&loaded, "b.bin");
    if (!entry || entry->asset.len != sizeof(b_data) || memcmp(entry->asset.data, b_data, sizeof(b_data)) != 0) {
        fprintf(stderr, "b.bin mismatch\n");
        mh_archive_free(&loaded);
        mh_buffer_free(&blob);
        mh_archive_free(&archive);
        return 1;
    }

    mh_archive_free(&loaded);
    mh_buffer_free(&blob);
    mh_archive_free(&archive);
    puts("layton pack roundtrip passed");
    return 0;
}
