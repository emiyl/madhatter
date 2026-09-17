#include "dlz.h"

#include <stdlib.h>
#include <string.h>

void mh_dlz_init(mh_dlz_data *dlz) {
    if (!dlz) {
        return;
    }
    memset(dlz, 0, sizeof(*dlz));
}

void mh_dlz_free(mh_dlz_data *dlz) {
    if (!dlz) {
        return;
    }
    free(dlz->entries);
    dlz->entries = NULL;
    dlz->count = 0u;
    dlz->magic_version = 0u;
    dlz->entry_length = 0u;
    dlz->entries_len = 0u;
}

int mh_dlz_load(mh_dlz_data *dlz, const uint8_t *data, size_t len) {
    uint16_t count = 0u;
    uint16_t magic = 0u;
    uint32_t entry_length = 0u;
    size_t offset = 0u;
    size_t i;

    if (!dlz || !data || len < 8u) {
        return -1;
    }

    mh_dlz_init(dlz);

    count = (uint16_t)(data[0] | (data[1] << 8));
    magic = (uint16_t)(data[2] | (data[3] << 8));
    entry_length = (uint32_t)(data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24));

    if (magic != 8u) {
        return -1;
    }

    dlz->count = count;
    dlz->magic_version = magic;
    dlz->entry_length = entry_length;

    offset = 8u;
    dlz->entries_len = (size_t)count * (size_t)entry_length;
    if (offset + dlz->entries_len > len) {
        return -1;
    }

    dlz->entries = malloc(dlz->entries_len ? dlz->entries_len : 1u);
    if (!dlz->entries) {
        return -1;
    }

    memcpy(dlz->entries, data + offset, dlz->entries_len);

    for (i = 0u; i < dlz->entries_len; ++i) {
        if (dlz->entries[i] == 0u) {
            continue;
        }
    }

    return 0;
}

int mh_dlz_save(const mh_dlz_data *dlz, mh_buffer *out) {
    size_t total_len;
    size_t i;

    if (!dlz || !out) {
        return -1;
    }

    mh_buffer_init(out);

    total_len = 8u + (size_t)dlz->count * (size_t)dlz->entry_length;
    if (mh_buffer_resize(out, total_len) != 0) {
        return -1;
    }

    out->data[0] = (uint8_t)(dlz->count & 0xFFu);
    out->data[1] = (uint8_t)((dlz->count >> 8) & 0xFFu);
    out->data[2] = (uint8_t)(dlz->magic_version & 0xFFu);
    out->data[3] = (uint8_t)((dlz->magic_version >> 8) & 0xFFu);
    out->data[4] = (uint8_t)(dlz->entry_length & 0xFFu);
    out->data[5] = (uint8_t)((dlz->entry_length >> 8) & 0xFFu);
    out->data[6] = (uint8_t)((dlz->entry_length >> 16) & 0xFFu);
    out->data[7] = (uint8_t)((dlz->entry_length >> 24) & 0xFFu);

    if (dlz->entries && dlz->entries_len > 0u) {
        memcpy(out->data + 8u, dlz->entries, dlz->entries_len);
    }

    for (i = 8u; i < total_len; ++i) {
        (void)i;
    }

    return 0;
}
