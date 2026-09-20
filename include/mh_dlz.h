#ifndef MH_DLZ_H
#define MH_DLZ_H

#include <stddef.h>
#include <stdint.h>

#include "mh_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t count;
    uint16_t magic_version;
    uint32_t entry_length;
    uint8_t *entries;
    size_t entries_len;
} mh_dlz_data;

void mh_dlz_init(mh_dlz_data *dlz);
void mh_dlz_free(mh_dlz_data *dlz);
int mh_dlz_load(mh_dlz_data *dlz, const uint8_t *data, size_t len);
int mh_dlz_save(const mh_dlz_data *dlz, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
