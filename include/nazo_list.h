#ifndef MH_NAZO_LIST_H
#define MH_NAZO_LIST_H

#include <stddef.h>
#include <stdint.h>

#include "mh_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t id_internal;
    uint16_t id_external;
    char name[48];
    int16_t id_group;
} mh_nazo_list_entry;

typedef struct {
    mh_nazo_list_entry *entries;
    size_t count;
} mh_nazo_list_data;

void mh_nazo_list_init(mh_nazo_list_data *list);
void mh_nazo_list_free(mh_nazo_list_data *list);
int mh_nazo_list_load(mh_nazo_list_data *list, const uint8_t *data, size_t len, int is_hd);
int mh_nazo_list_save(const mh_nazo_list_data *list, mh_buffer *out, int is_hd);

#ifdef __cplusplus
}
#endif

#endif
