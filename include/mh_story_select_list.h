#ifndef MH_STORY_SELECT_LIST_H
#define MH_STORY_SELECT_LIST_H

#include <stddef.h>
#include <stdint.h>

#include "mh_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char name[32];
    uint16_t id_entry;
    uint16_t goal;
    uint16_t index_place;
    int16_t id_event;
    int16_t id_connected;

    int16_t event_viewed_flags[4];
    size_t event_viewed_count;

    int16_t story_flags[4];
    size_t story_flag_count;

    int16_t puzzle_indexes[4];
    int16_t puzzle_states[4];
    size_t puzzle_count;

    int16_t event_counter_indexes[4];
    uint16_t event_counter_values[4];
    size_t event_counter_count;
} mh_story_select_entry;

typedef struct {
    mh_story_select_entry *entries;
    size_t count;
} mh_story_select_list_data;

void mh_story_select_list_init(mh_story_select_list_data *data);
void mh_story_select_list_free(mh_story_select_list_data *data);
int mh_story_select_list_load(mh_story_select_list_data *data, const uint8_t *blob, size_t len);
int mh_story_select_list_save(const mh_story_select_list_data *data, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
