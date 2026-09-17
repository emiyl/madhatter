#ifndef MH_EVENT_BASE_LIST_H
#define MH_EVENT_BASE_LIST_H

#include <stddef.h>
#include <stdint.h>

#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t id_event;
    int16_t index_puzzle;
    int16_t index_event_viewed_flag;
} mh_event_base_list_entry;

typedef struct {
    mh_event_base_list_entry *entries;
    size_t count;
} mh_event_base_list_data;

void mh_event_base_list_init(mh_event_base_list_data *data);
void mh_event_base_list_free(mh_event_base_list_data *data);
int mh_event_base_list_load(mh_event_base_list_data *data, const uint8_t *blob, size_t len);
int mh_event_base_list_save(const mh_event_base_list_data *data, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
