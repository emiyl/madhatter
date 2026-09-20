#include "mh_event_descriptor_bank.h"

#include <stdlib.h>
#include <string.h>

static void mh_strncpy_term(char *dst, size_t dst_size, const char *src) {
    size_t len = 0u;

    if (!dst || dst_size == 0u) {
        return;
    }
    memset(dst, 0, dst_size);
    if (!src) {
        return;
    }

    len = strlen(src);
    if (len >= dst_size) {
        len = dst_size - 1u;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
}

static int mh_event_descriptor_bank_entry_from_bytes(mh_event_descriptor_bank_entry *entry, const uint8_t *src, size_t len, int is_hd) {
    size_t desc_len = is_hd ? 64u : 48u;

    if (!entry || !src || len < (is_hd ? 68u : 52u)) {
        return -1;
    }

    entry->id_event = (uint32_t)(src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24));

    char tmp[65] = {0};
    for (size_t i = 0u; i < desc_len; ++i) {
        tmp[i] = (char)src[4u + i];
        if (src[4u + i] == 0u) {
            tmp[i] = '\0';
            break;
        }
    }
    tmp[desc_len] = '\0';
    mh_strncpy_term(entry->description, sizeof(entry->description), tmp);
    return 0;
}

static int mh_event_descriptor_bank_entry_to_bytes(const mh_event_descriptor_bank_entry *entry, mh_writer *out, int is_hd) {
    size_t desc_len = is_hd ? 64u : 48u;
    uint8_t desc[64] = {0};
    size_t copy_len = 0u;

    if (!entry || !out) {
        return -1;
    }

    copy_len = strlen(entry->description);
    if (copy_len > desc_len) {
        copy_len = desc_len;
    }
    memset(desc, 0, sizeof(desc));
    for (size_t i = 0u; i < copy_len; ++i) {
        desc[i] = (uint8_t)entry->description[i];
    }

    if (mh_writer_write_u32_le(out, entry->id_event) != 0) {
        return -1;
    }
    if (mh_writer_write_bytes(out, desc, desc_len) != 0) {
        return -1;
    }
    return 0;
}

void mh_event_descriptor_bank_init(mh_event_descriptor_bank_data *data) {
    if (!data) {
        return;
    }
    memset(data, 0, sizeof(*data));
}

void mh_event_descriptor_bank_free(mh_event_descriptor_bank_data *data) {
    if (!data) {
        return;
    }
    free(data->entries);
    data->entries = NULL;
    data->count = 0u;
}

int mh_event_descriptor_bank_load(mh_event_descriptor_bank_data *data, const uint8_t *blob, size_t len, int is_hd) {
    size_t entry_len = is_hd ? 68u : 52u;
    size_t count = 0u;
    uint16_t magic = 0u;
    uint32_t stored_len = 0u;
    mh_reader reader;

    if (!data || !blob || len < 8u) {
        return -1;
    }

    mh_event_descriptor_bank_init(data);

    count = (size_t)(uint16_t)(blob[0] | (blob[1] << 8));
    magic = (uint16_t)(blob[2] | (blob[3] << 8));
    stored_len = (uint32_t)(blob[4] | (blob[5] << 8) | (blob[6] << 16) | (blob[7] << 24));

    if (magic != 8u || stored_len != entry_len) {
        return -1;
    }
    if (count > (len - 8u) / entry_len) {
        return -1;
    }

    data->entries = calloc(count ? count : 1u, sizeof(*data->entries));
    if (!data->entries) {
        return -1;
    }
    data->count = count;

    mh_reader_init(&reader, blob + 8u, len - 8u);
    for (size_t i = 0u; i < count; ++i) {
        if (reader.pos + entry_len > reader.len) {
            mh_event_descriptor_bank_free(data);
            return -1;
        }
        if (mh_event_descriptor_bank_entry_from_bytes(&data->entries[i], reader.data + reader.pos, entry_len, is_hd) != 0) {
            mh_event_descriptor_bank_free(data);
            return -1;
        }
        reader.pos += entry_len;
    }
    return 0;
}

int mh_event_descriptor_bank_save(const mh_event_descriptor_bank_data *data, mh_buffer *out, int is_hd) {
    mh_writer writer = {0};
    size_t entry_len = is_hd ? 68u : 52u;

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
    if (mh_writer_write_u32_le(&writer, (uint32_t)entry_len) != 0) {
        mh_writer_free(&writer);
        return -1;
    }
    for (size_t i = 0u; i < data->count; ++i) {
        if (mh_event_descriptor_bank_entry_to_bytes(&data->entries[i], &writer, is_hd) != 0) {
            mh_writer_free(&writer);
            return -1;
        }
    }

    out->data = writer.data;
    out->len = writer.len;
    return 0;
}
