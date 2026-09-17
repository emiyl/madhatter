#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nazo_list.h"

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
    mh_nazo_list_data src = {0};
    mh_nazo_list_data loaded = {0};
    mh_buffer blob = {0};
    mh_nazo_list_entry entry1 = {0};
    mh_nazo_list_entry entry2 = {0};

    entry1.id_internal = 10u;
    entry1.id_external = 20u;
    snprintf(entry1.name, sizeof(entry1.name), "Puzzle A");
    entry1.id_group = 3;

    entry2.id_internal = 11u;
    entry2.id_external = 21u;
    snprintf(entry2.name, sizeof(entry2.name), "Puzzle B");
    entry2.id_group = -2;

    src.count = 2u;
    src.entries = calloc(src.count, sizeof(*src.entries));
    if (!src.entries) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }
    src.entries[0] = entry1;
    src.entries[1] = entry2;

    if (mh_nazo_list_save(&src, &blob, 0) != 0) {
        fprintf(stderr, "save failed\n");
        free(src.entries);
        return 1;
    }

    if (mh_nazo_list_load(&loaded, blob.data, blob.len, 0) != 0) {
        fprintf(stderr, "load failed\n");
        mh_buffer_free(&blob);
        free(src.entries);
        return 1;
    }

    expect_int("entry count", (int)loaded.count, 2);
    expect_int("id_internal0", (int)loaded.entries[0].id_internal, 10);
    expect_int("id_external0", (int)loaded.entries[0].id_external, 20);
    expect_int("id_group0", (int)loaded.entries[0].id_group, 3);
    expect_str("name0", loaded.entries[0].name, "Puzzle A");
    expect_int("id_internal1", (int)loaded.entries[1].id_internal, 11);
    expect_int("id_external1", (int)loaded.entries[1].id_external, 21);
    expect_int("id_group1", (int)loaded.entries[1].id_group, -2);
    expect_str("name1", loaded.entries[1].name, "Puzzle B");

    mh_nazo_list_free(&loaded);
    mh_buffer_free(&blob);
    free(src.entries);
    puts("nazo_list roundtrip passed");
    return 0;
}
