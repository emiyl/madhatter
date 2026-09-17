#include "time_definition.h"

#include <stdlib.h>
#include <string.h>

void mh_time_definition_init(mh_time_definition_data *data) {
    if (!data) {
        return;
    }
    memset(data, 0, sizeof(*data));
}

void mh_time_definition_free(mh_time_definition_data *data) {
    if (!data) {
        return;
    }
    free(data->entries);
    data->entries = NULL;
    data->count = 0u;
}

int mh_time_definition_load(mh_time_definition_data *data, const uint8_t *blob, size_t len) {
    size_t count = 0u;
    size_t total = 0u;
    mh_reader reader;

    if (!data || !blob || len < 8u) {
        return -1;
    }

    mh_time_definition_init(data);

    count = (size_t)(uint16_t)(blob[0] | (blob[1] << 8));
    if ((uint16_t)(blob[2] | (blob[3] << 8)) != 8u) {
        return -1;
    }
    if ((uint32_t)(blob[4] | (blob[5] << 8) | (blob[6] << 16) | (blob[7] << 24)) != 4u) {
        return -1;
    }

    total = count * 4u;
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
        if (reader.pos + 4u > reader.len) {
            mh_time_definition_free(data);
            return -1;
        }
        data->entries[i].id_time = (uint16_t)(reader.data[reader.pos] | (reader.data[reader.pos + 1] << 8));
        data->entries[i].count_frames = (uint16_t)(reader.data[reader.pos + 2] | (reader.data[reader.pos + 3] << 8));
        reader.pos += 4u;
    }
    return 0;
}

int mh_time_definition_save(const mh_time_definition_data *data, mh_buffer *out) {
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
    if (mh_writer_write_u32_le(&writer, 4u) != 0) {
        mh_writer_free(&writer);
        return -1;
    }
    for (size_t i = 0u; i < data->count; ++i) {
        if (mh_writer_write_u16_le(&writer, data->entries[i].id_time) != 0) {
            mh_writer_free(&writer);
            return -1;
        }
        if (mh_writer_write_u16_le(&writer, data->entries[i].count_frames) != 0) {
            mh_writer_free(&writer);
            return -1;
        }
    }

    out->data = writer.data;
    out->len = writer.len;
    return 0;
}
