#ifndef MH_EVENT_DESCRIPTOR_BANK_H
#define MH_EVENT_DESCRIPTOR_BANK_H

#include <stddef.h>
#include <stdint.h>

#include "mh_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t id_event;
    char description[64];
} mh_event_descriptor_bank_entry;

typedef struct {
    mh_event_descriptor_bank_entry *entries;
    size_t count;
} mh_event_descriptor_bank_data;

void mh_event_descriptor_bank_init(mh_event_descriptor_bank_data *data);
void mh_event_descriptor_bank_free(mh_event_descriptor_bank_data *data);
int mh_event_descriptor_bank_load(mh_event_descriptor_bank_data *data, const uint8_t *blob, size_t len, int is_hd);
int mh_event_descriptor_bank_save(const mh_event_descriptor_bank_data *data, mh_buffer *out, int is_hd);

#ifdef __cplusplus
}
#endif

#endif
