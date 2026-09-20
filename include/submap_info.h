#ifndef MH_SUBMAP_INFO_H
#define MH_SUBMAP_INFO_H

#include <stddef.h>
#include <stdint.h>

#include "mh_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t required_viewed_event_flag;
    uint8_t index_place;
    uint16_t chapter;
    uint8_t index_image;
    uint16_t x;
    uint16_t y;
    int is_hd;
} mh_submap_info_entry;

typedef struct {
    mh_submap_info_entry *entries;
    size_t count;
} mh_submap_info_data;

void mh_submap_info_init(mh_submap_info_data *data);
void mh_submap_info_free(mh_submap_info_data *data);
int mh_submap_info_load(mh_submap_info_data *data, const uint8_t *blob, size_t len, int is_hd);
int mh_submap_info_save(const mh_submap_info_data *data, mh_buffer *out, int is_hd);

#ifdef __cplusplus
}
#endif

#endif
