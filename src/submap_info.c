#include "mh_submap_info.h"

#include <stdlib.h>
#include <string.h>

static size_t mh_submap_entry_size(int is_hd) {
    return is_hd ? 12u : 8u;
}

void mh_submap_info_init(mh_submap_info_data *data) {
    if (!data) {
        return;
    }
    memset(data, 0, sizeof(*data));
}

void mh_submap_info_free(mh_submap_info_data *data) {
    if (!data) {
        return;
    }
    free(data->entries);
    data->entries = NULL;
    data->count = 0u;
}

int mh_submap_info_load(mh_submap_info_data *data, const uint8_t *blob, size_t len, int is_hd) {
    size_t count = 0u;
    size_t entry_len = 0u;
    size_t total = 0u;
    mh_reader reader;

    if (!data || !blob || len < 8u) {
        return -1;
    }

    mh_submap_info_init(data);

    count = (size_t)(uint16_t)(blob[0] | (blob[1] << 8));
    if ((uint16_t)(blob[2] | (blob[3] << 8)) != 8u) {
        return -1;
    }
    entry_len = mh_submap_entry_size(is_hd);
    if ((uint32_t)(blob[4] | (blob[5] << 8) | (blob[6] << 16) | (blob[7] << 24)) != entry_len) {
        return -1;
    }

    total = count * entry_len;
    if (len < 8u + total) {
        return -1;
    }

    data->entries = calloc(count ? count : 1u, sizeof(*data->entries));
    if (!data->entries) {
        return -1;
    }
    data->count = count;

    mh_reader_init(&reader, blob + 8u, len - 8u);
    for (size_t i = 0u; i < count; ++i) {
        mh_submap_info_entry *entry = &data->entries[i];

        if (reader.pos + entry_len > reader.len) {
            mh_submap_info_free(data);
            return -1;
        }

        memset(entry, 0, sizeof(*entry));
        entry->is_hd = is_hd;
        entry->required_viewed_event_flag = reader.data[reader.pos];
        entry->index_place = reader.data[reader.pos + 1];
        entry->chapter = (uint16_t)(reader.data[reader.pos + 2] | (reader.data[reader.pos + 3] << 8));
        entry->index_image = reader.data[reader.pos + 4];

        if (is_hd) {
            entry->x = (uint16_t)(reader.data[reader.pos + 6] | (reader.data[reader.pos + 7] << 8));
            entry->y = (uint16_t)(reader.data[reader.pos + 8] | (reader.data[reader.pos + 9] << 8));
        } else {
            entry->x = (uint16_t)reader.data[reader.pos + 5];
            entry->y = (uint16_t)reader.data[reader.pos + 6];
        }

        reader.pos += entry_len;
    }
    return 0;
}

int mh_submap_info_save(const mh_submap_info_data *data, mh_buffer *out, int is_hd) {
    mh_writer writer = {0};
    size_t entry_len = 0u;

    if (!data || !out) {
        return -1;
    }

    mh_buffer_init(out);
    mh_writer_init(&writer);

    entry_len = mh_submap_entry_size(is_hd);

    if (mh_writer_write_u16_le(&writer, (uint16_t)data->count) != 0) {
        mh_writer_free(&writer);
        return -1;
    }
    if (mh_writer_write_u16_le(&writer, 8u) != 0) {
        mh_writer_free(&writer);
        return -1;
    }
    if (mh_writer_write_u32_le(&writer, (uint32_t)entry_len) != 0) {
        mh_writer_free(&writer);
        return -1;
    }

    for (size_t i = 0u; i < data->count; ++i) {
        const mh_submap_info_entry *entry = &data->entries[i];
        uint8_t raw[12] = {0};

        raw[0] = entry->required_viewed_event_flag;
        raw[1] = entry->index_place;
        raw[2] = (uint8_t)(entry->chapter & 0xFFu);
        raw[3] = (uint8_t)((entry->chapter >> 8) & 0xFFu);
        raw[4] = entry->index_image;

        if (is_hd) {
            raw[5] = 0u;
            raw[6] = (uint8_t)(entry->x & 0xFFu);
            raw[7] = (uint8_t)((entry->x >> 8) & 0xFFu);
            raw[8] = (uint8_t)(entry->y & 0xFFu);
            raw[9] = (uint8_t)((entry->y >> 8) & 0xFFu);
            raw[10] = 0u;
            raw[11] = 0u;
        } else {
            raw[5] = (uint8_t)(entry->x & 0xFFu);
            raw[6] = (uint8_t)(entry->y & 0xFFu);
            raw[7] = 0u;
        }

        if (mh_writer_write_bytes(&writer, raw, entry_len) != 0) {
            mh_writer_free(&writer);
            return -1;
        }
    }

    out->data = writer.data;
    out->len = writer.len;
    return 0;
}
