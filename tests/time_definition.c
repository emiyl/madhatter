#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "time_definition.h"

static void expect_int(const char *label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "%s: got %d want %d\n", label, got, want);
        exit(1);
    }
}

int main(void) {
    mh_time_definition_data src = {0};
    mh_time_definition_data loaded = {0};
    mh_buffer blob = {0};

    src.entries = calloc(2u, sizeof(*src.entries));
    src.count = 2u;
    src.entries[0].id_time = 10u;
    src.entries[0].count_frames = 33u;
    src.entries[1].id_time = 11u;
    src.entries[1].count_frames = 44u;

    if (mh_time_definition_save(&src, &blob) != 0) {
        fprintf(stderr, "save failed\n");
        free(src.entries);
        return 1;
    }
    if (mh_time_definition_load(&loaded, blob.data, blob.len) != 0) {
        fprintf(stderr, "load failed\n");
        mh_buffer_free(&blob);
        free(src.entries);
        return 1;
    }

    expect_int("count", (int)loaded.count, 2);
    expect_int("id_time_0", (int)loaded.entries[0].id_time, 10);
    expect_int("count_frames_0", (int)loaded.entries[0].count_frames, 33);
    expect_int("id_time_1", (int)loaded.entries[1].id_time, 11);
    expect_int("count_frames_1", (int)loaded.entries[1].count_frames, 44);

    mh_time_definition_free(&loaded);
    mh_buffer_free(&blob);
    free(src.entries);
    puts("time_definition roundtrip passed");
    return 0;
}
