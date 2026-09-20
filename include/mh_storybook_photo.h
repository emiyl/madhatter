#ifndef MH_STORYBOOK_PHOTO_H
#define MH_STORYBOOK_PHOTO_H

#include <stddef.h>
#include <stdint.h>

#include "mh_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t id_photo;
    uint16_t value;
} mh_storybook_photo_entry;

typedef struct {
    mh_storybook_photo_entry *entries;
    size_t count;
} mh_storybook_photo_data;

void mh_storybook_photo_init(mh_storybook_photo_data *data);
void mh_storybook_photo_free(mh_storybook_photo_data *data);
int mh_storybook_photo_load(mh_storybook_photo_data *data, const uint8_t *blob, size_t len);
int mh_storybook_photo_save(const mh_storybook_photo_data *data, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
