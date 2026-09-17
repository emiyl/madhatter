#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "story_select_list.h"

static void expect_int(const char *label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "%s: got %d want %d\n", label, got, want);
        exit(1);
    }
}

static void expect_str(const char *label, const char *got, const char *want) {
    if (strcmp(got, want) != 0) {
        fprintf(stderr, "%s: got '%s' want '%s'\n", label, got, want);
        exit(1);
    }
}

int main(void) {
    mh_story_select_list_data src = {0};
    mh_story_select_list_data loaded = {0};
    mh_buffer blob = {0};
    mh_story_select_entry entry = {0};

    snprintf(entry.name, sizeof(entry.name), "Story A");
    entry.id_entry = 7u;
    entry.goal = 20u;
    entry.index_place = 3u;
    entry.id_event = -1;
    entry.id_connected = 9;
    entry.event_viewed_flags[0] = 10;
    entry.event_viewed_count = 1u;
    entry.story_flags[0] = 11;
    entry.story_flag_count = 1u;
    entry.puzzle_indexes[0] = 12;
    entry.puzzle_states[0] = 2;
    entry.puzzle_count = 1u;
    entry.event_counter_indexes[0] = 13;
    entry.event_counter_values[0] = 99u;
    entry.event_counter_count = 1u;

    src.count = 1u;
    src.entries = calloc(src.count, sizeof(*src.entries));
    if (!src.entries) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }
    src.entries[0] = entry;

    if (mh_story_select_list_save(&src, &blob) != 0) {
        fprintf(stderr, "save failed\n");
        free(src.entries);
        return 1;
    }

    if (mh_story_select_list_load(&loaded, blob.data, blob.len) != 0) {
        fprintf(stderr, "load failed\n");
        mh_buffer_free(&blob);
        free(src.entries);
        return 1;
    }

    expect_int("count", (int)loaded.count, 1);
    expect_str("name", loaded.entries[0].name, "Story A");
    expect_int("id_entry", (int)loaded.entries[0].id_entry, 7);
    expect_int("goal", (int)loaded.entries[0].goal, 20);
    expect_int("index_place", (int)loaded.entries[0].index_place, 3);
    expect_int("id_event", (int)loaded.entries[0].id_event, -1);
    expect_int("id_connected", (int)loaded.entries[0].id_connected, 9);
    expect_int("event_viewed_count", (int)loaded.entries[0].event_viewed_count, 1);
    expect_int("story_flag_count", (int)loaded.entries[0].story_flag_count, 1);
    expect_int("puzzle_count", (int)loaded.entries[0].puzzle_count, 1);
    expect_int("event_counter_count", (int)loaded.entries[0].event_counter_count, 1);

    mh_story_select_list_free(&loaded);
    mh_buffer_free(&blob);
    free(src.entries);
    puts("story_select_list roundtrip passed");
    return 0;
}
