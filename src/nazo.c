#include "nazo.h"

#include <stdlib.h>
#include <string.h>

#define MH_NAZO_HEADER_ALIGN 112u

static size_t mh_nazo_strlen(const char *s) {
    size_t len = 0u;
    if (!s) {
        return 0u;
    }
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

static int mh_nazo_write_padded_string(mh_writer *writer, const char *s, size_t length) {
    size_t i = 0u;
    size_t len = 0u;

    if (!writer || !s) {
        return -1;
    }

    len = mh_nazo_strlen(s);
    if (len > length) {
        len = length;
    }

    for (i = 0u; i < len; ++i) {
        if (mh_writer_write_u8(writer, (uint8_t)s[i]) != 0) {
            return -1;
        }
    }
    for (; i < length; ++i) {
        if (mh_writer_write_u8(writer, 0u) != 0) {
            return -1;
        }
    }
    return 0;
}

static int mh_nazo_read_padded_string(mh_reader *reader, size_t length, char **out) {
    uint8_t *bytes = NULL;
    size_t i = 0u;
    size_t trim = 0u;

    if (!reader || !out) {
        return -1;
    }

    if (reader->pos + length > reader->len) {
        *out = NULL;
        return -1;
    }

    bytes = (uint8_t *)malloc(length + 1u);
    if (!bytes) {
        return -1;
    }

    for (i = 0u; i < length; ++i) {
        bytes[i] = mh_reader_read_u8(reader);
    }
    bytes[length] = '\0';

    trim = length;
    while (trim > 0u && bytes[trim - 1u] == 0u) {
        trim--;
    }

    if (trim == 0u) {
        free(bytes);
        *out = malloc(1u);
        if (!*out) {
            return -1;
        }
        (*out)[0] = '\0';
        return 0;
    }

    *out = malloc(trim + 1u);
    if (!*out) {
        free(bytes);
        return -1;
    }
    memcpy(*out, bytes, trim);
    (*out)[trim] = '\0';
    free(bytes);
    return 0;
}

static int mh_nazo_read_string_at(const uint8_t *data, size_t len, size_t offset, char **out) {
    size_t cursor = offset;
    size_t start = offset;

    if (!data || !out) {
        return -1;
    }
    *out = NULL;
    if (offset >= len) {
        return -1;
    }

    while (cursor < len && data[cursor] != 0u) {
        cursor++;
    }
    if (cursor == len) {
        return -1;
    }

    if (cursor == start) {
        *out = malloc(1u);
        if (!*out) {
            return -1;
        }
        (*out)[0] = '\0';
        return 0;
    }

    *out = malloc((cursor - start) + 1u);
    if (!*out) {
        return -1;
    }
    memcpy(*out, data + start, cursor - start);
    (*out)[cursor - start] = '\0';
    return 0;
}

static int mh_nazo_align(mh_writer *writer, size_t alignment) {
    size_t extra = 0u;
    if (!writer) {
        return -1;
    }
    extra = (alignment - (writer->len % alignment)) % alignment;
    while (extra-- > 0u) {
        if (mh_writer_write_u8(writer, 0u) != 0) {
            return -1;
        }
    }
    return 0;
}

static void mh_nazo_common_init(mh_nazo_data *nazo) {
    if (!nazo) {
        return;
    }
    memset(nazo, 0, sizeof(*nazo));
    nazo->id_reward = -1;
}

static int mh_nazo_write_bank_string(mh_writer *bank, const char *text, size_t max_len,
                                     uint32_t *offset_out, uint32_t *last_valid_offset) {
    size_t len = 0u;

    if (!bank || !offset_out || !last_valid_offset) {
        return -1;
    }

    *offset_out = *last_valid_offset;
    if (!text || text[0] == '\0') {
        return 0;
    }

    len = mh_nazo_strlen(text) + 1u;
    if (bank->len + len > max_len) {
        return 0;
    }

    *offset_out = (uint32_t)bank->len;
    *last_valid_offset = *offset_out;
    if (mh_writer_write_bytes(bank, (const uint8_t *)text, len) != 0) {
        return -1;
    }
    return 0;
}

void mh_nazo_init(mh_nazo_data *nazo) {
    mh_nazo_common_init(nazo);
}

void mh_nazo_free(mh_nazo_data *nazo) {
    if (!nazo) {
        return;
    }
    free(nazo->text_name);
    free(nazo->text_prompt);
    free(nazo->text_correct);
    free(nazo->text_incorrect);
    for (size_t i = 0u; i < 3u; ++i) {
        free(nazo->text_hint[i]);
        nazo->text_hint[i] = NULL;
    }
    mh_nazo_common_init(nazo);
}

static int mh_nazo_load_common(mh_nazo_data *nazo, const uint8_t *data, size_t len, int is_hd) {
    mh_reader reader;
    uint32_t offsets[7] = {0u};
    size_t bank_base = MH_NAZO_HEADER_ALIGN;
    size_t name_length = is_hd ? 72u : 48u;
    size_t i = 0u;

    if (!nazo || !data || len < 64u) {
        return -1;
    }

    mh_nazo_common_init(nazo);
    mh_reader_init(&reader, data, len);
    nazo->id_external = mh_reader_read_u16_le(&reader);

    if (is_hd) {
        mh_reader_seek(&reader, 4u);
    } else {
        nazo->length_header = mh_reader_read_u16_le(&reader);
    }

    if (mh_nazo_read_padded_string(&reader, name_length, &nazo->text_name) != 0) {
        return -1;
    }

    nazo->id_tutorial = mh_reader_read_u8(&reader);
    nazo->picarat_decay_stages[0] = mh_reader_read_u8(&reader);
    nazo->picarat_decay_stages[1] = mh_reader_read_u8(&reader);
    nazo->picarat_decay_stages[2] = mh_reader_read_u8(&reader);
    nazo->flags = mh_reader_read_u8(&reader);
    nazo->use_luke_solver = (nazo->flags & 0x01u) == 0u;
    nazo->use_luke_voicelines = (nazo->flags & 0x02u) != 0u;
    nazo->has_answer_background = (nazo->flags & 0x10u) != 0u;
    nazo->use_language_prompt_background = (nazo->flags & 0x20u) != 0u;
    nazo->use_language_answer_background = (nazo->flags & 0x40u) != 0u;
    nazo->index_place = mh_reader_read_u8(&reader);
    nazo->id_handler = mh_reader_read_u8(&reader);
    nazo->bg_main_id = mh_reader_read_u8(&reader);
    mh_reader_seek(&reader, mh_reader_tell(&reader) + 2u);
    nazo->bg_sub_id = mh_reader_read_u8(&reader);
    nazo->id_reward = (int8_t)mh_reader_read_u8(&reader);

    for (i = 0u; i < 7u; ++i) {
        offsets[i] = mh_reader_read_u32_le(&reader);
    }

    if (bank_base + offsets[0] < len) {
        mh_nazo_read_string_at(data, len, bank_base + offsets[0], &nazo->text_prompt);
    }
    if (bank_base + offsets[1] < len) {
        mh_nazo_read_string_at(data, len, bank_base + offsets[1], &nazo->text_correct);
    }
    if (bank_base + offsets[2] < len) {
        mh_nazo_read_string_at(data, len, bank_base + offsets[2], &nazo->text_incorrect);
    }
    for (i = 0u; i < 3u; ++i) {
        if (bank_base + offsets[3u + i] < len) {
            mh_nazo_read_string_at(data, len, bank_base + offsets[3u + i], &nazo->text_hint[i]);
        }
    }

    return 0;
}

int mh_nazo_load_nds(mh_nazo_data *nazo, const uint8_t *data, size_t len) {
    return mh_nazo_load_common(nazo, data, len, 0);
}

int mh_nazo_load_hd(mh_nazo_data *nazo, const uint8_t *data, size_t len) {
    return mh_nazo_load_common(nazo, data, len, 1);
}

static int mh_nazo_save_common(const mh_nazo_data *nazo, mh_buffer *out, int is_hd) {
    mh_writer writer = {0};
    mh_writer bank = {0};
    uint32_t offsets[7] = {0u};
    uint32_t last_valid = 0u;
    size_t max_len = is_hd ? 4136u : 2560u;
    const char *texts[7] = {0};
    size_t header_name_len = is_hd ? 72u : 48u;
    uint8_t flags = 0u;

    if (!nazo || !out) {
        return -1;
    }

    texts[0] = nazo->text_prompt ? nazo->text_prompt : "";
    texts[1] = nazo->text_correct ? nazo->text_correct : "";
    texts[2] = nazo->text_incorrect ? nazo->text_incorrect : "";
    texts[3] = nazo->text_hint[0] ? nazo->text_hint[0] : "";
    texts[4] = nazo->text_hint[1] ? nazo->text_hint[1] : "";
    texts[5] = nazo->text_hint[2] ? nazo->text_hint[2] : "";
    texts[6] = "";

    mh_writer_init(&writer);
    mh_writer_init(&bank);
    mh_buffer_init(out);

    if (mh_writer_write_u16_le(&writer, nazo->id_external) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (is_hd) {
        if (mh_writer_write_u16_le(&writer, 0u) != 0) {
            mh_writer_free(&writer);
            mh_writer_free(&bank);
            return -1;
        }
    } else {
        if (mh_writer_write_u16_le(&writer, 112u) != 0) {
            mh_writer_free(&writer);
            mh_writer_free(&bank);
            return -1;
        }
    }
    if (mh_nazo_write_padded_string(&writer, nazo->text_name ? nazo->text_name : "", header_name_len) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }

    if (mh_writer_write_u8(&writer, nazo->id_tutorial) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (mh_writer_write_u8(&writer, nazo->picarat_decay_stages[0]) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (mh_writer_write_u8(&writer, nazo->picarat_decay_stages[1]) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (mh_writer_write_u8(&writer, nazo->picarat_decay_stages[2]) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }

    flags = 0u;
    if (!nazo->use_luke_solver) {
        flags |= 0x01u;
    }
    if (nazo->use_luke_voicelines) {
        flags |= 0x02u;
    }
    if (nazo->has_answer_background) {
        flags |= 0x10u;
    }
    if (nazo->use_language_prompt_background) {
        flags |= 0x20u;
    }
    if (nazo->use_language_answer_background) {
        flags |= 0x40u;
    }
    if (mh_writer_write_u8(&writer, flags) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (mh_writer_write_u8(&writer, nazo->index_place) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (mh_writer_write_u8(&writer, nazo->id_handler) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (mh_writer_write_u8(&writer, nazo->bg_main_id) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (mh_writer_write_u8(&writer, 0u) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (mh_writer_write_u8(&writer, 0u) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (mh_writer_write_u8(&writer, nazo->bg_sub_id) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (mh_writer_write_u8(&writer, (uint8_t)nazo->id_reward) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }

    for (size_t i = 0u; i < 7u; ++i) {
        if (mh_nazo_write_bank_string(&bank, texts[i], max_len, &offsets[i], &last_valid) != 0) {
            mh_writer_free(&writer);
            mh_writer_free(&bank);
            return -1;
        }
    }

    for (size_t i = 0u; i < 7u; ++i) {
        if (mh_writer_write_u32_le(&writer, offsets[i]) != 0) {
            mh_writer_free(&writer);
            mh_writer_free(&bank);
            return -1;
        }
    }

    if (mh_nazo_align(&writer, MH_NAZO_HEADER_ALIGN) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (mh_writer_write_bytes(&writer, bank.data, bank.len) != 0) {
        mh_writer_free(&writer);
        mh_writer_free(&bank);
        return -1;
    }
    if (is_hd) {
        while ((writer.len % 4248u) != 0u) {
            if (mh_writer_write_u8(&writer, 0u) != 0) {
                mh_writer_free(&writer);
                mh_writer_free(&bank);
                return -1;
            }
        }
    }

    out->data = writer.data;
    out->len = writer.len;
    mh_writer_free(&bank);
    return 0;
}

int mh_nazo_save_nds(const mh_nazo_data *nazo, mh_buffer *out) {
    return mh_nazo_save_common(nazo, out, 0);
}

int mh_nazo_save_hd(const mh_nazo_data *nazo, mh_buffer *out) {
    return mh_nazo_save_common(nazo, out, 1);
}
