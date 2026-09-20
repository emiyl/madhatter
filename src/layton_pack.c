#include "mh_layton_pack.h"

#include <stdlib.h>
#include <string.h>

#define MH_LAYTON_PACK_HEADER_SIZE 16u
#define MH_LAYTON_PACK_METADATA_SIZE 16u

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

static uint32_t mh_align4(uint32_t v) {
    return (v + 3u) & ~3u;
}

static int mh_read_padded_name(const uint8_t *src, size_t src_len, size_t offset, size_t name_len, char **out_name) {
    size_t i;
    size_t end;
    char *name;

    if (!src || !out_name || offset >= src_len || name_len == 0u) {
        return -1;
    }

    end = name_len;
    for (i = 0u; i < name_len; ++i) {
        if (offset + i >= src_len) {
            return -1;
        }
        if (src[offset + i] == '\0') {
            end = i;
            break;
        }
    }

    name = malloc(end + 1u);
    if (!name) {
        return -1;
    }

    for (i = 0u; i < end; ++i) {
        name[i] = (char)src[offset + i];
    }
    name[end] = '\0';
    *out_name = name;
    return 0;
}

int mh_archive_load_layton_pack(mh_archive *archive, const uint8_t *data, size_t len, int version) {
    mh_reader reader;
    uint32_t offset_header;
    uint32_t length_archive;
    uint32_t file_count = 0u;

    if (!archive || !data || len < MH_LAYTON_PACK_HEADER_SIZE) {
        return -1;
    }

    mh_archive_init(archive);
    mh_reader_init(&reader, data, len);

    offset_header = mh_reader_read_u32_le(&reader);
    length_archive = mh_reader_read_u32_le(&reader);
    if (version == 0) {
        mh_reader_seek(&reader, 4u);
    }

    if (offset_header == 0u || offset_header >= len || length_archive == 0u || length_archive > len) {
        mh_archive_free(archive);
        return -1;
    }

    mh_reader_seek(&reader, offset_header);
    while (reader.pos < length_archive) {
        uint32_t metadata[4];
        uint32_t name_len;
        uint32_t payload_len;
        uint32_t record_start;
        uint32_t name_offset;
        uint32_t payload_offset;
        uint32_t next_record_offset;
        char *name = NULL;
        uint8_t *payload = NULL;
        size_t i;

        if (reader.pos + 16u > len) {
            mh_archive_free(archive);
            return -1;
        }

        record_start = (uint32_t)reader.pos;
        for (i = 0u; i < 4u; ++i) {
            metadata[i] = mh_reader_read_u32_le(&reader);
        }

        name_len = metadata[0] - MH_LAYTON_PACK_METADATA_SIZE;
        payload_len = metadata[3];
        if (payload_len == 0u) {
            break;
        }
        if (name_len == 0u) {
            mh_archive_free(archive);
            return -1;
        }

        name_offset = record_start + 16u;
        payload_offset = record_start + metadata[0];
        if (name_offset + name_len > len || payload_offset + payload_len > len) {
            mh_archive_free(archive);
            return -1;
        }

        if (mh_read_padded_name(data, len, name_offset, name_len, &name) != 0) {
            mh_archive_free(archive);
            return -1;
        }

        payload = malloc(payload_len ? payload_len : 1u);
        if (!payload) {
            free(name);
            mh_archive_free(archive);
            return -1;
        }
        memcpy(payload, data + payload_offset, payload_len);

        if (mh_archive_add_file(archive, name, payload, payload_len) != 0) {
            free(payload);
            free(name);
            mh_archive_free(archive);
            return -1;
        }

        free(payload);
        free(name);

        next_record_offset = record_start + metadata[1];
        if (next_record_offset > len) {
            mh_archive_free(archive);
            return -1;
        }
        mh_reader_seek(&reader, next_record_offset);

        if (++file_count > 1024u) {
            mh_archive_free(archive);
            return -1;
        }
    }

    return 0;
}

int mh_archive_save_layton_pack(const mh_archive *archive, mh_buffer *out, int version) {
    mh_writer writer = {0};
    mh_writer header = {0};
    mh_writer data = {0};
    size_t i;

    if (!archive || !out) {
        return -1;
    }

    mh_buffer_init(out);

    if (mh_writer_write_u32_le(&writer, MH_LAYTON_PACK_HEADER_SIZE) != 0 ||
        mh_writer_write_u32_le(&writer, 0u) != 0) {
        return -1;
    }

    if (version == 1) {
        if (mh_writer_write_bytes(&writer, (const uint8_t *)"PCK2", 4u) != 0 ||
            mh_writer_write_u32_le(&writer, 0u) != 0) {
            return -1;
        }
    } else {
        if (mh_writer_write_u32_le(&writer, (uint32_t)archive->count) != 0 ||
            mh_writer_write_bytes(&writer, (const uint8_t *)"LPCK", 4u) != 0) {
            return -1;
        }
    }

    for (i = 0u; i < archive->count; ++i) {
        const mh_archive_entry *entry = &archive->entries[i];
        uint32_t name_block_len = mh_align4((uint32_t)(mh_name_len(entry->name) + 1u));
        uint32_t payload_len = (uint32_t)entry->asset.len;
        size_t j;
        uint32_t before_grid = 0u;
        uint32_t after_grid = 0u;

        mh_writer_init(&data);
        mh_writer_init(&header);

        if (mh_writer_write_bytes(&data, (const uint8_t *)entry->name, mh_name_len(entry->name)) != 0 ||
            mh_writer_write_u8(&data, 0u) != 0) {
            return -1;
        }
        for (j = (uint32_t)mh_name_len(entry->name) + 1u; j < name_block_len; ++j) {
            if (mh_writer_write_u8(&data, 0u) != 0) {
                return -1;
            }
        }
        if (mh_writer_write_bytes(&data, entry->asset.data, entry->asset.len) != 0) {
            return -1;
        }

        before_grid = (uint32_t)data.len;
        while ((data.len & 3u) != 0u) {
            if (mh_writer_write_u8(&data, 0u) != 0) {
                return -1;
            }
        }
        if (before_grid == data.len) {
            for (j = 0u; j < 4u; ++j) {
                if (mh_writer_write_u8(&data, 0u) != 0) {
                    return -1;
                }
            }
        }

        after_grid = (uint32_t)data.len;
        if (mh_writer_write_u32_le(&header, name_block_len + MH_LAYTON_PACK_METADATA_SIZE) != 0 ||
            mh_writer_write_u32_le(&header, after_grid + MH_LAYTON_PACK_METADATA_SIZE) != 0 ||
            mh_writer_write_u32_le(&header, 0u) != 0 ||
            mh_writer_write_u32_le(&header, payload_len) != 0) {
            return -1;
        }

        if (mh_writer_write_bytes(&writer, header.data, header.len) != 0 ||
            mh_writer_write_bytes(&writer, data.data, data.len) != 0) {
            return -1;
        }

        mh_writer_free(&header);
        mh_writer_free(&data);
    }

    if (writer.data && writer.len >= 8u) {
        writer.data[4] = (uint8_t)(writer.len & 0xFFu);
        writer.data[5] = (uint8_t)((writer.len >> 8) & 0xFFu);
        writer.data[6] = (uint8_t)((writer.len >> 16) & 0xFFu);
        writer.data[7] = (uint8_t)((writer.len >> 24) & 0xFFu);
    }

    out->data = writer.data;
    out->len = writer.len;
    return 0;
}
