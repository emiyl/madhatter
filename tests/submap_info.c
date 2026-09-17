#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "submap_info.h"

static void expect_int(const char *label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "%s: got %d want %d\n", label, got, want);
        exit(1);
    }
}

int main(void) {
    mh_submap_info_data src_nds = {0};
    mh_submap_info_data loaded_nds = {0};
    mh_submap_info_data src_hd = {0};
    mh_submap_info_data loaded_hd = {0};
    mh_buffer blob_nds = {0};
    mh_buffer blob_hd = {0};

    src_nds.entries = calloc(1u, sizeof(*src_nds.entries));
    src_nds.count = 1u;
    src_nds.entries[0].required_viewed_event_flag = 3u;
    src_nds.entries[0].index_place = 5u;
    src_nds.entries[0].chapter = 300u;
    src_nds.entries[0].index_image = 7u;
    src_nds.entries[0].x = 11u;
    src_nds.entries[0].y = 13u;

    src_hd.entries = calloc(1u, sizeof(*src_hd.entries));
    src_hd.count = 1u;
    src_hd.entries[0].required_viewed_event_flag = 9u;
    src_hd.entries[0].index_place = 2u;
    src_hd.entries[0].chapter = 400u;
    src_hd.entries[0].index_image = 8u;
    src_hd.entries[0].x = 0x1234u;
    src_hd.entries[0].y = 0x5678u;

    if (mh_submap_info_save(&src_nds, &blob_nds, 0) != 0) {
        fprintf(stderr, "save nds failed\n");
        free(src_nds.entries);
        return 1;
    }
    if (mh_submap_info_save(&src_hd, &blob_hd, 1) != 0) {
        fprintf(stderr, "save hd failed\n");
        mh_buffer_free(&blob_nds);
        free(src_nds.entries);
        free(src_hd.entries);
        return 1;
    }

    if (mh_submap_info_load(&loaded_nds, blob_nds.data, blob_nds.len, 0) != 0) {
        fprintf(stderr, "load nds failed\n");
        mh_buffer_free(&blob_nds);
        mh_buffer_free(&blob_hd);
        free(src_nds.entries);
        free(src_hd.entries);
        return 1;
    }
    if (mh_submap_info_load(&loaded_hd, blob_hd.data, blob_hd.len, 1) != 0) {
        fprintf(stderr, "load hd failed\n");
        mh_submap_info_free(&loaded_nds);
        mh_buffer_free(&blob_nds);
        mh_buffer_free(&blob_hd);
        free(src_nds.entries);
        free(src_hd.entries);
        return 1;
    }

    expect_int("nds count", (int)loaded_nds.count, 1);
    expect_int("nds required_viewed_event_flag", (int)loaded_nds.entries[0].required_viewed_event_flag, 3);
    expect_int("nds index_place", (int)loaded_nds.entries[0].index_place, 5);
    expect_int("nds chapter", (int)loaded_nds.entries[0].chapter, 300);
    expect_int("nds index_image", (int)loaded_nds.entries[0].index_image, 7);
    expect_int("nds x", (int)loaded_nds.entries[0].x, 11);
    expect_int("nds y", (int)loaded_nds.entries[0].y, 13);

    expect_int("hd count", (int)loaded_hd.count, 1);
    expect_int("hd required_viewed_event_flag", (int)loaded_hd.entries[0].required_viewed_event_flag, 9);
    expect_int("hd index_place", (int)loaded_hd.entries[0].index_place, 2);
    expect_int("hd chapter", (int)loaded_hd.entries[0].chapter, 400);
    expect_int("hd index_image", (int)loaded_hd.entries[0].index_image, 8);
    expect_int("hd x", (int)loaded_hd.entries[0].x, 0x1234);
    expect_int("hd y", (int)loaded_hd.entries[0].y, 0x5678);

    mh_submap_info_free(&loaded_nds);
    mh_submap_info_free(&loaded_hd);
    mh_buffer_free(&blob_nds);
    mh_buffer_free(&blob_hd);
    free(src_nds.entries);
    free(src_hd.entries);
    puts("submap_info roundtrip passed");
    return 0;
}
