#include "mh_layton_pack2.h"

#include <stdlib.h>
#include <string.h>

#define MH_LAYTON_PACK2_HEADER_SIZE 32u

static int mh_read_cstring_from_buffer(const uint8_t *src, size_t src_len, size_t offset, char **out_name) {
    size_t len = 0u;
    char *name = NULL;

    if (!src || !out_name || offset >= src_len) {
        return -1;
    }

    while (offset + len < src_len && src[offset + len] != '\0') {
        len += 1u;
    }

    if (offset + len >= src_len) {
        return -1;
    }

    name = malloc(len + 1u);
    if (!name) {
        return -1;
    }
    memcpy(name, src + offset, len);
    name[len] = '\0';
    *out_name = name;
    return 0;
}

static size_t mh_name_len(const char *s) {
    size_t len = 0u;
    if (!s) {
        return 0u;
    }
    while (s[len] != '\0') {
        len += 1u;
    }
    return len;
}

int mh_archive_load_layton_pack2(mh_archive *archive, const uint8_t *data, size_t len) {
    mh_reader reader;
    uint32_t count_file;
    uint32_t offset_file;
    uint32_t offset_metadata;
    uint32_t offset_name;
    uint32_t index;
    size_t magic_offset = 0u;
    int found_magic = 0;
    size_t candidate;

    if (!archive || !data || len < MH_LAYTON_PACK2_HEADER_SIZE) {
        return -1;
    }

    mh_archive_init(archive);
    mh_reader_init(&reader, data, len);

    for (candidate = 0u; candidate + 4u <= len && candidate < 32u; ++candidate) {
        if (memcmp(data + candidate, "LPC2", 4u) == 0 || memcmp(data + candidate, "PCK2", 4u) == 0) {
            magic_offset = candidate;
            found_magic = 1;
            break;
        }
    }

    if (!found_magic) {
        return -1;
    }

    mh_reader_seek(&reader, magic_offset + 4u);
    count_file = mh_reader_read_u32_le(&reader);
    offset_file = mh_reader_read_u32_le(&reader);
    mh_reader_read_u32_le(&reader);
    offset_metadata = mh_reader_read_u32_le(&reader);
    offset_name = mh_reader_read_u32_le(&reader);

    if (count_file == 0u || offset_file == 0u || offset_name == 0u || offset_metadata == 0u) {
        size_t shifted = magic_offset > 8u ? magic_offset - 8u : 0u;
        if (shifted + 20u <= len) {
            mh_reader_init(&reader, data + shifted, len - shifted);
            mh_reader_seek(&reader, 4u);
            count_file = mh_reader_read_u32_le(&reader);
            offset_file = mh_reader_read_u32_le(&reader);
            mh_reader_read_u32_le(&reader);
            offset_metadata = mh_reader_read_u32_le(&reader);
            offset_name = mh_reader_read_u32_le(&reader);
        }
    }

    if (count_file == 0u || offset_file == 0u || offset_name == 0u || offset_metadata == 0u ||
        offset_file > len || offset_metadata > len || offset_name > len) {
        mh_archive_free(archive);
        return -1;
    }

    for (index = 0u; index < count_file; ++index) {
        uint32_t file_offset_name;
        uint32_t file_offset_data;
        uint32_t file_length_data;
        char *name = NULL;
        uint32_t metadata_pos = offset_metadata + (index * 12u);
        uint32_t name_pos;
        const uint8_t *payload;

        if (metadata_pos + 12u > len) {
            mh_archive_free(archive);
            return -1;
        }

        mh_reader_init(&reader, data + metadata_pos, len - metadata_pos);
        file_offset_name = mh_reader_read_u32_le(&reader);
        file_offset_data = mh_reader_read_u32_le(&reader);
        file_length_data = mh_reader_read_u32_le(&reader);

        if (offset_name + file_offset_name >= len) {
            mh_archive_free(archive);
            return -1;
        }

        name_pos = offset_name + file_offset_name;
        if (mh_read_cstring_from_buffer(data, len, name_pos, &name) != 0) {
            mh_archive_free(archive);
            return -1;
        }

        if (offset_file + file_offset_data + file_length_data > len) {
            free(name);
            mh_archive_free(archive);
            return -1;
        }

        payload = data + offset_file + file_offset_data;
        if (mh_archive_add_file(archive, name, payload, file_length_data) != 0) {
            free(name);
            mh_archive_free(archive);
            return -1;
        }

        free(name);
    }

    return 0;
}

int mh_archive_save_layton_pack2(const mh_archive *archive, mh_buffer *out) {
    mh_writer metadata = {0};
    mh_writer section_name = {0};
    mh_writer section_data = {0};
    mh_writer writer = {0};
    uint32_t offset_file;
    uint32_t offset_metadata = MH_LAYTON_PACK2_HEADER_SIZE;
    uint32_t offset_name;
    size_t i;
    uint32_t total_len;

    if (!archive || !out) {
        return -1;
    }

    mh_buffer_init(out);
    mh_writer_init(&writer);

    for (i = 0u; i < archive->count; ++i) {
        const mh_archive_entry *entry = &archive->entries[i];
        size_t name_len = mh_name_len(entry->name);

        if (mh_writer_write_u32_le(&metadata, (uint32_t)section_name.len) != 0 ||
            mh_writer_write_u32_le(&metadata, (uint32_t)section_data.len) != 0 ||
            mh_writer_write_u32_le(&metadata, (uint32_t)entry->asset.len) != 0) {
            goto fail;
        }

        if (mh_writer_write_bytes(&section_name, (const uint8_t *)entry->name, name_len) != 0 ||
            mh_writer_write_u8(&section_name, 0u) != 0) {
            goto fail;
        }

        if (mh_writer_write_bytes(&section_data, entry->asset.data, entry->asset.len) != 0) {
            goto fail;
        }

        while ((section_data.len & 3u) != 0u) {
            if (mh_writer_write_u8(&section_data, 0u) != 0) {
                goto fail;
            }
        }
    }

    while ((section_name.len & 3u) != 0u) {
        if (mh_writer_write_u8(&section_name, 0u) != 0) {
            goto fail;
        }
    }

    offset_name = offset_metadata + metadata.len;
    offset_file = offset_name + section_name.len;
    total_len = offset_file + section_data.len + 4u;

    if (mh_writer_write_bytes(&writer, (const uint8_t *)"LPC2", 4u) != 0 ||
        mh_writer_write_u32_le(&writer, (uint32_t)archive->count) != 0 ||
        mh_writer_write_u32_le(&writer, offset_file) != 0 ||
        mh_writer_write_u32_le(&writer, 0u) != 0 ||
        mh_writer_write_u32_le(&writer, offset_metadata) != 0 ||
        mh_writer_write_u32_le(&writer, offset_name) != 0 ||
        mh_writer_write_u32_le(&writer, offset_file) != 0) {
        goto fail;
    }

    while (writer.len < MH_LAYTON_PACK2_HEADER_SIZE) {
        if (mh_writer_write_u8(&writer, 0u) != 0) {
            goto fail;
        }
    }

    if (mh_writer_write_bytes(&writer, metadata.data, metadata.len) != 0 ||
        mh_writer_write_bytes(&writer, section_name.data, section_name.len) != 0 ||
        mh_writer_write_bytes(&writer, section_data.data, section_data.len) != 0) {
        goto fail;
    }

    if (mh_writer_write_u32_le(&writer, total_len) != 0) {
        goto fail;
    }

    out->data = writer.data;
    out->len = writer.len;
    mh_writer_free(&metadata);
    mh_writer_free(&section_name);
    mh_writer_free(&section_data);
    return 0;

fail:
    mh_writer_free(&metadata);
    mh_writer_free(&section_name);
    mh_writer_free(&section_data);
    mh_writer_free(&writer);
    return -1;
}
