#include "event_info_list.h"

#include <stdlib.h>
#include <string.h>

static uint16_t mh_event_info_list_read_masked_u16(const uint8_t *ptr) {
    uint16_t v = (uint16_t)(ptr[0] | (ptr[1] << 8));
    if (v == 0xFFFFu) {
        return 0xFFFFu;
    }
    return v;
}

void mh_event_info_list_init(mh_event_info_list_data *data) {
    if (!data) {
        return;
    }
    memset(data, 0, sizeof(*data));
}

void mh_event_info_list_free(mh_event_info_list_data *data) {
    if (!data) {
        return;
    }
    free(data->entries);
    data->entries = NULL;
    data->count = 0u;
}

int mh_event_info_list_load(mh_event_info_list_data *data, const uint8_t *blob, size_t len) {
    size_t count = 0u;
    size_t total = 0u;
    mh_reader reader;

    if (!data || !blob || len < 8u) {
        return -1;
    }

    mh_event_info_list_init(data);

    count = (size_t)(uint16_t)(blob[0] | (blob[1] << 8));
    if ((uint16_t)(blob[2] | (blob[3] << 8)) != 8u) {
        return -1;
    }
    if ((uint32_t)(blob[4] | (blob[5] << 8) | (blob[6] << 16) | (blob[7] << 24)) != 12u) {
        return -1;
    }

    total = count * 12u;
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
        if (reader.pos + 12u > reader.len) {
            mh_event_info_list_free(data);
            return -1;
        }
        data->entries[i].id_event = mh_event_info_list_read_masked_u16(reader.data + reader.pos);
        data->entries[i].type_event = mh_event_info_list_read_masked_u16(reader.data + reader.pos + 2);
        data->entries[i].data_sound_set = mh_event_info_list_read_masked_u16(reader.data + reader.pos + 4);
        data->entries[i].data_puzzle = mh_event_info_list_read_masked_u16(reader.data + reader.pos + 6);
        data->entries[i].index_event_viewed_flag = mh_event_info_list_read_masked_u16(reader.data + reader.pos + 8);
        data->entries[i].index_story_flag = mh_event_info_list_read_masked_u16(reader.data + reader.pos + 10);
        reader.pos += 12u;
    }
    return 0;
}

int mh_event_info_list_save(const mh_event_info_list_data *data, mh_buffer *out) {
    mh_writer writer = {0};

    if (!data || !out) {
        return -1;
    }

    mh_buffer_init(out);
    mh_writer_init(&writer);

    if (mh_writer_write_u16_le(&writer, (uint16_t)data->count) != 0) {
        mh_writer_free(&writer);
        return -1;
    }
    if (mh_writer_write_u16_le(&writer, 8u) != 0) {
        mh_writer_free(&writer);
        return -1;
    }
    if (mh_writer_write_u32_le(&writer, 12u) != 0) {
        mh_writer_free(&writer);
        return -1;
    }
    for (size_t i = 0u; i < data->count; ++i) {
        uint16_t v[6] = {
            data->entries[i].id_event,
            data->entries[i].type_event,
            data->entries[i].data_sound_set,
            data->entries[i].data_puzzle,
            data->entries[i].index_event_viewed_flag,
            data->entries[i].index_story_flag,
        };
        for (size_t j = 0u; j < 6u; ++j) {
            if (mh_writer_write_u16_le(&writer, v[j]) != 0) {
                mh_writer_free(&writer);
                return -1;
            }
        }
    }

    out->data = writer.data;
    out->len = writer.len;
    return 0;
}
