#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "storybook_photo.h"

static void expect_int(const char *label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "%s: got %d want %d\n", label, got, want);
        exit(1);
    }
}

int main(void) {
    mh_storybook_photo_data src = {0};
    mh_storybook_photo_data loaded = {0};
    mh_buffer blob = {0};

    src.entries = calloc(2u, sizeof(*src.entries));
    src.count = 2u;
    src.entries[0].id_photo = 1u;
    src.entries[0].value = 0x1234u;
    src.entries[1].id_photo = 2u;
    src.entries[1].value = 0x5678u;

    if (mh_storybook_photo_save(&src, &blob) != 0) {
        fprintf(stderr, "save failed\n");
        free(src.entries);
        return 1;
    }
    if (mh_storybook_photo_load(&loaded, blob.data, blob.len) != 0) {
        fprintf(stderr, "load failed\n");
        mh_buffer_free(&blob);
        free(src.entries);
        return 1;
    }

    expect_int("count", (int)loaded.count, 2);
    expect_int("id_photo_0", (int)loaded.entries[0].id_photo, 1);
    expect_int("value_0", (int)loaded.entries[0].value, 0x1234);
    expect_int("id_photo_1", (int)loaded.entries[1].id_photo, 2);
    expect_int("value_1", (int)loaded.entries[1].value, 0x5678);

    mh_storybook_photo_free(&loaded);
    mh_buffer_free(&blob);
    free(src.entries);
    puts("storybook_photo roundtrip passed");
    return 0;
}
