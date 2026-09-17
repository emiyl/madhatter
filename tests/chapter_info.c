#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "chapter_info.h"

static void expect_int(const char *label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "%s: got %d want %d\n", label, got, want);
        exit(1);
    }
}

int main(void) {
    mh_chapter_info_data src = {0};
    mh_chapter_info_data loaded = {0};
    mh_buffer blob = {0};

    src.entries = calloc(2u, sizeof(*src.entries));
    src.count = 2u;
    src.entries[0].chapter = 1u;
    src.entries[0].id_event = 101u;
    src.entries[0].index_event_viewed_flag = 3u;
    src.entries[0].id_event_alt = 202u;
    src.entries[1].chapter = 2u;
    src.entries[1].id_event = 103u;
    src.entries[1].index_event_viewed_flag = 7u;
    src.entries[1].id_event_alt = 204u;

    if (mh_chapter_info_save(&src, &blob) != 0) {
        fprintf(stderr, "save failed\n");
        free(src.entries);
        return 1;
    }
    if (mh_chapter_info_load(&loaded, blob.data, blob.len) != 0) {
        fprintf(stderr, "load failed\n");
        mh_buffer_free(&blob);
        free(src.entries);
        return 1;
    }

    expect_int("count", (int)loaded.count, 2);
    expect_int("chapter_0", (int)loaded.entries[0].chapter, 1);
    expect_int("id_event_0", (int)loaded.entries[0].id_event, 101);
    expect_int("index_event_viewed_flag_0", (int)loaded.entries[0].index_event_viewed_flag, 3);
    expect_int("id_event_alt_0", (int)loaded.entries[0].id_event_alt, 202);
    expect_int("chapter_1", (int)loaded.entries[1].chapter, 2);
    expect_int("id_event_1", (int)loaded.entries[1].id_event, 103);
    expect_int("index_event_viewed_flag_1", (int)loaded.entries[1].index_event_viewed_flag, 7);
    expect_int("id_event_alt_1", (int)loaded.entries[1].id_event_alt, 204);

    mh_chapter_info_free(&loaded);
    mh_buffer_free(&blob);
    free(src.entries);
    puts("chapter_info roundtrip passed");
    return 0;
}
