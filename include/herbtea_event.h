#ifndef MH_HERBTEA_EVENT_H
#define MH_HERBTEA_EVENT_H

#include <stddef.h>
#include <stdint.h>

#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t id_event;
    uint16_t id_herbtea_flag;
} mh_herbtea_event_entry;

typedef struct {
    mh_herbtea_event_entry *entries;
    size_t count;
} mh_herbtea_event_data;

void mh_herbtea_event_init(mh_herbtea_event_data *data);
void mh_herbtea_event_free(mh_herbtea_event_data *data);
int mh_herbtea_event_load(mh_herbtea_event_data *data, const uint8_t *blob, size_t len);
int mh_herbtea_event_save(const mh_herbtea_event_data *data, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
