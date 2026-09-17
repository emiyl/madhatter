#ifndef MH_GOAL_INFO_H
#define MH_GOAL_INFO_H

#include <stddef.h>
#include <stdint.h>

#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t id_event;
    uint16_t type;
    uint16_t goal;
} mh_goal_info_entry;

typedef struct {
    mh_goal_info_entry *entries;
    size_t count;
} mh_goal_info_data;

void mh_goal_info_init(mh_goal_info_data *data);
void mh_goal_info_free(mh_goal_info_data *data);
int mh_goal_info_load(mh_goal_info_data *data, const uint8_t *blob, size_t len);
int mh_goal_info_save(const mh_goal_info_data *data, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
