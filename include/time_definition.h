#ifndef MH_TIME_DEFINITION_H
#define MH_TIME_DEFINITION_H

#include <stddef.h>
#include <stdint.h>

#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t id_time;
    uint16_t count_frames;
} mh_time_definition_entry;

typedef struct {
    mh_time_definition_entry *entries;
    size_t count;
} mh_time_definition_data;

void mh_time_definition_init(mh_time_definition_data *data);
void mh_time_definition_free(mh_time_definition_data *data);
int mh_time_definition_load(mh_time_definition_data *data, const uint8_t *blob, size_t len);
int mh_time_definition_save(const mh_time_definition_data *data, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
