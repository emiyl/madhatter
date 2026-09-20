#include "mh_nazo_list.h"

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

static int mh_nazo_list_entry_from_bytes(mh_nazo_list_entry *entry, const uint8_t *src, size_t len) {
    size_t name_len = 0u;
    char tmp[81] = {0};

    if (!entry || !src || len < 54u) {
        return -1;
    }

    entry->id_internal = (uint16_t)(src[0] | (src[1] << 8));
    entry->id_external = (uint16_t)(src[2] | (src[3] << 8));

    name_len = (len >= 86u) ? 80u : 48u;

    for (size_t i = 0u; i < name_len; ++i) {
        tmp[i] = (char)src[4u + i];
        if (src[4u + i] == 0u) {
            tmp[i] = '\0';
            break;
        }
    }
    tmp[name_len] = '\0';

    entry->id_group = (int16_t)(src[4u + name_len] | (src[5u + name_len] << 8));
    mh_strncpy_term(entry->name, sizeof(entry->name), tmp);
    return 0;
}

static int mh_nazo_list_entry_to_bytes(const mh_nazo_list_entry *entry, mh_writer *out, int is_hd) {
    size_t name_len = is_hd ? 80u : 48u;
    uint8_t name_bytes[80] = {0};
    size_t copy_len = 0u;

    if (!entry || !out) {
        return -1;
    }

    copy_len = strlen(entry->name);
    if (copy_len > name_len) {
        copy_len = name_len;
    }

    memset(name_bytes, 0, sizeof(name_bytes));
    for (size_t i = 0u; i < copy_len; ++i) {
        name_bytes[i] = (uint8_t)entry->name[i];
    }

    if (mh_writer_write_u16_le(out, entry->id_internal) != 0) return -1;
    if (mh_writer_write_u16_le(out, entry->id_external) != 0) return -1;
    if (mh_writer_write_bytes(out, name_bytes, name_len) != 0) return -1;
    if (mh_writer_write_u16_le(out, (uint16_t)(int16_t)entry->id_group) != 0) return -1;
    return 0;
}

void mh_nazo_list_init(mh_nazo_list_data *list) {
    if (!list) {
        return;
    }
    memset(list, 0, sizeof(*list));
}

void mh_nazo_list_free(mh_nazo_list_data *list) {
    if (!list) {
        return;
    }
    free(list->entries);
    list->entries = NULL;
    list->count = 0u;
}

int mh_nazo_list_load(mh_nazo_list_data *list, const uint8_t *data, size_t len, int is_hd) {
    size_t entry_len = is_hd ? 0x56u : 0x36u;
    size_t count = 0u;
    uint16_t magic = 0u;
    uint32_t stored_len = 0u;
    mh_reader reader;

    if (!list || !data || len < 8u) {
        return -1;
    }

    mh_nazo_list_init(list);

    count = (size_t)(uint16_t)(data[0] | (data[1] << 8));
    magic = (uint16_t)(data[2] | (data[3] << 8));
    stored_len = (uint32_t)(data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24));

    if (magic != 8u || stored_len != entry_len) {
        return -1;
    }

    if (count > (len - 8u) / entry_len) {
        return -1;
    }

    list->entries = calloc(count ? count : 1u, sizeof(*list->entries));
    if (!list->entries) {
        return -1;
    }
    list->count = count;

    mh_reader_init(&reader, data + 8u, len - 8u);
    for (size_t i = 0u; i < count; ++i) {
        if (reader.pos + entry_len > reader.len) {
            mh_nazo_list_free(list);
            return -1;
        }
        if (mh_nazo_list_entry_from_bytes(&list->entries[i], reader.data + reader.pos, entry_len) != 0) {
            mh_nazo_list_free(list);
            return -1;
        }
        reader.pos += entry_len;
    }
    return 0;
}

int mh_nazo_list_save(const mh_nazo_list_data *list, mh_buffer *out, int is_hd) {
    mh_writer writer = {0};
    size_t entry_len = is_hd ? 0x56u : 0x36u;

    if (!list || !out) {
        return -1;
    }

    mh_buffer_init(out);
    mh_writer_init(&writer);
    if (mh_writer_write_u16_le(&writer, (uint16_t)list->count) != 0) {
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
    for (size_t i = 0u; i < list->count; ++i) {
        if (mh_nazo_list_entry_to_bytes(&list->entries[i], &writer, is_hd) != 0) {
            mh_writer_free(&writer);
            return -1;
        }
    }

    out->data = writer.data;
    out->len = writer.len;
    return 0;
}
