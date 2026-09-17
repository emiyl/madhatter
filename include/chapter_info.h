#ifndef MH_CHAPTER_INFO_H
#define MH_CHAPTER_INFO_H

#include <stddef.h>
#include <stdint.h>

#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t chapter;
    uint16_t id_event;
    uint16_t index_event_viewed_flag;
    uint16_t id_event_alt;
} mh_chapter_info_entry;

typedef struct {
    mh_chapter_info_entry *entries;
    size_t count;
} mh_chapter_info_data;

void mh_chapter_info_init(mh_chapter_info_data *data);
void mh_chapter_info_free(mh_chapter_info_data *data);
int mh_chapter_info_load(mh_chapter_info_data *data, const uint8_t *blob, size_t len);
int mh_chapter_info_save(const mh_chapter_info_data *data, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
