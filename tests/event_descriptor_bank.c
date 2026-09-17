#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "event_descriptor_bank.h"

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
    mh_event_descriptor_bank_data src = {0};
    mh_event_descriptor_bank_data loaded = {0};
    mh_buffer blob = {0};
    mh_event_descriptor_bank_entry entry1 = {0};
    mh_event_descriptor_bank_entry entry2 = {0};

    entry1.id_event = 123u;
    snprintf(entry1.description, sizeof(entry1.description), "Event A");
    entry2.id_event = 456u;
    snprintf(entry2.description, sizeof(entry2.description), "Event B");

    src.count = 2u;
    src.entries = calloc(src.count, sizeof(*src.entries));
    if (!src.entries) {
        fprintf(stderr, "alloc failed\n");
        return 1;
    }
    src.entries[0] = entry1;
    src.entries[1] = entry2;

    if (mh_event_descriptor_bank_save(&src, &blob, 0) != 0) {
        fprintf(stderr, "save failed\n");
        free(src.entries);
        return 1;
    }

    if (mh_event_descriptor_bank_load(&loaded, blob.data, blob.len, 0) != 0) {
        fprintf(stderr, "load failed\n");
        mh_buffer_free(&blob);
        free(src.entries);
        return 1;
    }

    expect_int("count", (int)loaded.count, 2);
    expect_int("id_event_0", (int)loaded.entries[0].id_event, 123);
    expect_str("description_0", loaded.entries[0].description, "Event A");
    expect_int("id_event_1", (int)loaded.entries[1].id_event, 456);
    expect_str("description_1", loaded.entries[1].description, "Event B");

    mh_event_descriptor_bank_free(&loaded);
    mh_buffer_free(&blob);
    free(src.entries);
    puts("event_descriptor_bank roundtrip passed");
    return 0;
}
