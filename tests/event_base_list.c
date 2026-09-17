#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "event_base_list.h"

static void expect_int(const char *label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "%s: got %d want %d\n", label, got, want);
        exit(1);
    }
}

int main(void) {
    mh_event_base_list_data src = {0};
    mh_event_base_list_data loaded = {0};
    mh_buffer blob = {0};
    mh_event_base_list_entry entry1 = {0};
    mh_event_base_list_entry entry2 = {0};

    entry1.id_event = 10u;
    entry1.index_puzzle = -5;
    entry1.index_event_viewed_flag = 42;

    entry2.id_event = 11u;
    entry2.index_puzzle = 7;
    entry2.index_event_viewed_flag = -9;

    src.count = 2u;
    src.entries = calloc(src.count, sizeof(*src.entries));
    if (!src.entries) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }
    src.entries[0] = entry1;
    src.entries[1] = entry2;

    if (mh_event_base_list_save(&src, &blob) != 0) {
        fprintf(stderr, "save failed\n");
        free(src.entries);
        return 1;
    }

    if (mh_event_base_list_load(&loaded, blob.data, blob.len) != 0) {
        fprintf(stderr, "load failed\n");
        mh_buffer_free(&blob);
        free(src.entries);
        return 1;
    }

    expect_int("count", (int)loaded.count, 2);
    expect_int("id_event_0", (int)loaded.entries[0].id_event, 10);
    expect_int("index_puzzle_0", (int)loaded.entries[0].index_puzzle, -5);
    expect_int("index_event_viewed_flag_0", (int)loaded.entries[0].index_event_viewed_flag, 42);
    expect_int("id_event_1", (int)loaded.entries[1].id_event, 11);
    expect_int("index_puzzle_1", (int)loaded.entries[1].index_puzzle, 7);
    expect_int("index_event_viewed_flag_1", (int)loaded.entries[1].index_event_viewed_flag, -9);

    mh_event_base_list_free(&loaded);
    mh_buffer_free(&blob);
    free(src.entries);
    puts("event_base_list roundtrip passed");
    return 0;
}
