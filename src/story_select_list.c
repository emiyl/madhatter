#include "story_select_list.h"

#include <stdlib.h>
#include <string.h>

static int mh_read_s16_at(const uint8_t *src, size_t off) {
    return (int16_t)(src[off] | (src[off + 1] << 8));
}

static uint16_t mh_read_u16_at(const uint8_t *src, size_t off) {
    return (uint16_t)(src[off] | (src[off + 1] << 8));
}

void mh_story_select_list_init(mh_story_select_list_data *data) {
    if (!data) {
        return;
    }
    memset(data, 0, sizeof(*data));
}

void mh_story_select_list_free(mh_story_select_list_data *data) {
    if (!data) {
        return;
    }
    free(data->entries);
    data->entries = NULL;
    data->count = 0u;
}

int mh_story_select_list_load(mh_story_select_list_data *data, const uint8_t *blob, size_t len) {
    size_t count = 0u;
    size_t total = 0u;
    mh_reader reader;

    if (!data || !blob || len < 8u) {
        return -1;
    }

    mh_story_select_list_init(data);

    count = (size_t)(uint16_t)(blob[0] | (blob[1] << 8));
    if ((uint16_t)(blob[2] | (blob[3] << 8)) != 8u) {
        return -1;
    }
    if ((uint32_t)(blob[4] | (blob[5] << 8) | (blob[6] << 16) | (blob[7] << 24)) != 90u) {
        return -1;
    }

    total = count * 90u;
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
        mh_story_select_entry *e = &data->entries[i];
        const uint8_t *p = reader.data + reader.pos;
        size_t pos = 0u;

        if (reader.pos + 90u > reader.len) {
            mh_story_select_list_free(data);
            return -1;
        }

        memset(e, 0, sizeof(*e));

        for (size_t j = 0u; j < 32u; ++j) {
            e->name[j] = (char)p[j];
            if (p[j] == 0u) {
                break;
            }
        }
        e->name[31] = '\0';

        e->id_entry = mh_read_u16_at(p, 32u);
        e->goal = mh_read_u16_at(p, 34u);
        e->index_place = mh_read_u16_at(p, 36u);
        e->id_event = mh_read_s16_at(p, 38u);
        e->id_connected = mh_read_s16_at(p, 40u);

        pos = 42u;
        for (size_t j = 0u; j < 4u; ++j) {
            int16_t v = mh_read_s16_at(p, pos);
            if (v != -1) {
                e->event_viewed_flags[e->event_viewed_count++] = v;
            } else {
                break;
            }
            pos += 2u;
        }

        pos = 50u;
        for (size_t j = 0u; j < 4u; ++j) {
            int16_t v = mh_read_s16_at(p, pos);
            if (v != -1) {
                e->story_flags[e->story_flag_count++] = v;
            } else {
                break;
            }
            pos += 2u;
        }

        pos = 58u;
        for (size_t j = 0u; j < 4u; ++j) {
            int16_t index = mh_read_s16_at(p, pos);
            int16_t state = mh_read_s16_at(p, pos + 2u);
            if (index != -1) {
                e->puzzle_indexes[e->puzzle_count] = index;
                e->puzzle_states[e->puzzle_count] = (state == 1) ? 2 : ((state == 0) ? 1 : state);
                e->puzzle_count += 1u;
            } else {
                break;
            }
            pos += 4u;
        }

        pos = 74u;
        for (size_t j = 0u; j < 4u; ++j) {
            int16_t index = mh_read_s16_at(p, pos);
            uint16_t value = mh_read_u16_at(p, pos + 2u);
            if (index != -1 && index >= 0 && index < 128) {
                e->event_counter_indexes[e->event_counter_count] = index;
                e->event_counter_values[e->event_counter_count] = value;
                e->event_counter_count += 1u;
            } else {
                break;
            }
            pos += 4u;
        }

        reader.pos += 90u;
    }
    return 0;
}

int mh_story_select_list_save(const mh_story_select_list_data *data, mh_buffer *out) {
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
    if (mh_writer_write_u32_le(&writer, 90u) != 0) {
        mh_writer_free(&writer);
        return -1;
    }
    for (size_t i = 0u; i < data->count; ++i) {
        const mh_story_select_entry *e = &data->entries[i];
        uint8_t scratch[90] = {0};
        size_t pos = 0u;

        memset(scratch, 0, sizeof(scratch));
        for (size_t j = 0u; j < 32u; ++j) {
            scratch[j] = (uint8_t)e->name[j];
        }
        pos = 32u;
        scratch[pos++] = (uint8_t)(e->id_entry & 0xFFu);
        scratch[pos++] = (uint8_t)((e->id_entry >> 8) & 0xFFu);
        scratch[pos++] = (uint8_t)(e->goal & 0xFFu);
        scratch[pos++] = (uint8_t)((e->goal >> 8) & 0xFFu);
        scratch[pos++] = (uint8_t)(e->index_place & 0xFFu);
        scratch[pos++] = (uint8_t)((e->index_place >> 8) & 0xFFu);
        scratch[pos++] = (uint8_t)(e->id_event & 0xFFu);
        scratch[pos++] = (uint8_t)((e->id_event >> 8) & 0xFFu);
        scratch[pos++] = (uint8_t)(e->id_connected & 0xFFu);
        scratch[pos++] = (uint8_t)((e->id_connected >> 8) & 0xFFu);

        for (size_t j = 0u; j < 4u; ++j) {
            int16_t v = (j < e->event_viewed_count) ? e->event_viewed_flags[j] : -1;
            scratch[pos++] = (uint8_t)(v & 0xFFu);
            scratch[pos++] = (uint8_t)((v >> 8) & 0xFFu);
        }

        pos = 50u;
        for (size_t j = 0u; j < 4u; ++j) {
            int16_t v = (j < e->story_flag_count) ? e->story_flags[j] : -1;
            scratch[pos++] = (uint8_t)(v & 0xFFu);
            scratch[pos++] = (uint8_t)((v >> 8) & 0xFFu);
        }

        pos = 58u;
        for (size_t j = 0u; j < 4u; ++j) {
            int16_t idx = (j < e->puzzle_count) ? e->puzzle_indexes[j] : -1;
            int16_t state = (j < e->puzzle_count) ? e->puzzle_states[j] : 0;
            scratch[pos++] = (uint8_t)(idx & 0xFFu);
            scratch[pos++] = (uint8_t)((idx >> 8) & 0xFFu);
            scratch[pos++] = (uint8_t)(state & 0xFFu);
            scratch[pos++] = (uint8_t)((state >> 8) & 0xFFu);
        }

        pos = 74u;
        for (size_t j = 0u; j < 4u; ++j) {
            int16_t idx = (j < e->event_counter_count) ? e->event_counter_indexes[j] : -1;
            uint16_t value = (j < e->event_counter_count) ? e->event_counter_values[j] : 0u;
            scratch[pos++] = (uint8_t)(idx & 0xFFu);
            scratch[pos++] = (uint8_t)((idx >> 8) & 0xFFu);
            scratch[pos++] = (uint8_t)(value & 0xFFu);
            scratch[pos++] = (uint8_t)((value >> 8) & 0xFFu);
        }

        if (mh_writer_write_bytes(&writer, scratch, sizeof(scratch)) != 0) {
            mh_writer_free(&writer);
            return -1;
        }
    }

    out->data = writer.data;
    out->len = writer.len;
    return 0;
}
