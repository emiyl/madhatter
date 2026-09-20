#ifndef MH_EVENT_INFO_LIST_H
#define MH_EVENT_INFO_LIST_H

#include <stddef.h>
#include <stdint.h>

#include "mh_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t id_event;
    uint16_t type_event;
    uint16_t data_sound_set;
    uint16_t data_puzzle;
    uint16_t index_event_viewed_flag;
    uint16_t index_story_flag;
} mh_event_info_entry;

typedef struct {
    mh_event_info_entry *entries;
    size_t count;
} mh_event_info_list_data;

void mh_event_info_list_init(mh_event_info_list_data *data);
void mh_event_info_list_free(mh_event_info_list_data *data);
int mh_event_info_list_load(mh_event_info_list_data *data, const uint8_t *blob, size_t len);
int mh_event_info_list_save(const mh_event_info_list_data *data, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
