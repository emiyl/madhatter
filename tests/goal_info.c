#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "goal_info.h"

static void expect_int(const char *label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "%s: got %d want %d\n", label, got, want);
        exit(1);
    }
}

int main(void) {
    mh_goal_info_data src = {0};
    mh_goal_info_data loaded = {0};
    mh_buffer blob = {0};
    mh_goal_info_entry entry1 = {0};
    mh_goal_info_entry entry2 = {0};

    entry1.id_event = 10u;
    entry1.type = 1u;
    entry1.goal = 42u;

    entry2.id_event = 11u;
    entry2.type = 0u;
    entry2.goal = 99u;

    src.count = 2u;
    src.entries = calloc(src.count, sizeof(*src.entries));
    if (!src.entries) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }
    src.entries[0] = entry1;
    src.entries[1] = entry2;

    if (mh_goal_info_save(&src, &blob) != 0) {
        fprintf(stderr, "save failed\n");
        free(src.entries);
        return 1;
    }

    if (mh_goal_info_load(&loaded, blob.data, blob.len) != 0) {
        fprintf(stderr, "load failed\n");
        mh_buffer_free(&blob);
        free(src.entries);
        return 1;
    }

    expect_int("count", (int)loaded.count, 2);
    expect_int("id_event_0", (int)loaded.entries[0].id_event, 10);
    expect_int("type_0", (int)loaded.entries[0].type, 1);
    expect_int("goal_0", (int)loaded.entries[0].goal, 42);
    expect_int("id_event_1", (int)loaded.entries[1].id_event, 11);
    expect_int("type_1", (int)loaded.entries[1].type, 0);
    expect_int("goal_1", (int)loaded.entries[1].goal, 99);

    mh_goal_info_free(&loaded);
    mh_buffer_free(&blob);
    free(src.entries);
    puts("goal_info roundtrip passed");
    return 0;
}
