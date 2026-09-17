#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dlz.h"

int main(void) {
    mh_dlz_data src = {0};
    mh_dlz_data loaded = {0};
    mh_buffer blob = {0};
    const uint8_t entry_a[] = {'A', 'B', 'C'};
    const uint8_t entry_b[] = {'D', 'E', 'F'};
    const uint8_t entries[] = {'A', 'B', 'C', 'D', 'E', 'F'};
    uint8_t *ptr = NULL;

    src.count = 2u;
    src.magic_version = 8u;
    src.entry_length = 3u;
    src.entries_len = sizeof(entries);
    src.entries = malloc(src.entries_len);
    if (!src.entries) {
        fprintf(stderr, "dlz allocation failed\n");
        return 1;
    }
    memcpy(src.entries, entries, src.entries_len);

    if (mh_dlz_save(&src, &blob) != 0) {
        fprintf(stderr, "dlz save failed\n");
        free(src.entries);
        return 1;
    }

    if (mh_dlz_load(&loaded, blob.data, blob.len) != 0) {
        fprintf(stderr, "dlz load failed\n");
        mh_buffer_free(&blob);
        free(src.entries);
        return 1;
    }

    if (loaded.count != 2u || loaded.entry_length != 3u || loaded.entries_len != sizeof(entries)) {
        fprintf(stderr, "dlz metadata mismatch\n");
        mh_dlz_free(&loaded);
        mh_buffer_free(&blob);
        free(src.entries);
        return 1;
    }

    ptr = loaded.entries;
    if (memcmp(ptr, entry_a, sizeof(entry_a)) != 0 || memcmp(ptr + 3, entry_b, sizeof(entry_b)) != 0) {
        fprintf(stderr, "dlz payload mismatch\n");
        mh_dlz_free(&loaded);
        mh_buffer_free(&blob);
        free(src.entries);
        return 1;
    }

    mh_dlz_free(&loaded);
    mh_buffer_free(&blob);
    free(src.entries);
    puts("dlz roundtrip passed");
    return 0;
}
