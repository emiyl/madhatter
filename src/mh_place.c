#include "mh_place.h"

#include <string.h>
#include <stdio.h>

#include "stream.h"

#define MH_PLACE_OFF_POS_MAP 24u
#define MH_PLACE_HINTCOIN_COUNT 4u
#define MH_PLACE_HINTCOIN_SIZE 4u
#define MH_PLACE_TOBJ_COUNT 16u
#define MH_PLACE_TOBJ_SIZE 10u
#define MH_PLACE_BGANI_COUNT 12u
#define MH_PLACE_BGANI_SIZE 32u
#define MH_PLACE_EVENT_COUNT 16u
#define MH_PLACE_EVENT_SIZE 8u
#define MH_PLACE_EXIT_SIZE 12u
#define MH_PLACE_TAIL_PAD 48u

int mh_place_exit_can_spawn_event(const mh_place_exit *exit) {
    if (!exit) {
        return 0;
    }
    return exit->mode_decoding >= 2;
}

static int mh_place_bounding_is_empty(const mh_bounding_box *b) {
    return b->x == 0 && b->y == 0 && b->width == 0 && b->height == 0;
}

int mh_place_load_nds(mh_place_data *place, const uint8_t *data, size_t len) {
    mh_reader reader;
    size_t i;

    if (!place || !data) {
        fprintf(stderr, "madhatter: Invalid arguments to mh_place_load_nds\n");
        return -1;
    }
    memset(place, 0, sizeof(*place));

    mh_reader_init(&reader, data, len);

    place->id_name_place = mh_reader_read_u8(&reader);
    mh_reader_seek(&reader, MH_PLACE_OFF_POS_MAP);
    if (!mh_reader_has_data(&reader)) {
        fprintf(stderr, "madhatter: Failed to read map position from place data\n");
        return -1;
    }

    place->pos_map_x = mh_reader_read_u8(&reader);
    place->pos_map_y = mh_reader_read_u8(&reader);
    place->bg_main_id = mh_reader_read_u8(&reader);
    place->bg_map_id = mh_reader_read_u8(&reader);

    for (i = 0; i < MH_PLACE_HINTCOIN_COUNT; ++i) {
        uint8_t x, y, w, h;
        if (reader.pos + MH_PLACE_HINTCOIN_SIZE > len) {
            fprintf(stderr, "madhatter: Not enough data to read hint coin %zu\n", i);
            return -1;
        }
        x = mh_reader_read_u8(&reader);
        y = mh_reader_read_u8(&reader);
        w = mh_reader_read_u8(&reader);
        h = mh_reader_read_u8(&reader);
        if (x == 0u && y == 0u && w == 0u && h == 0u) {
            mh_reader_seek(&reader, reader.pos + ((MH_PLACE_HINTCOIN_COUNT - i - 1u) * MH_PLACE_HINTCOIN_SIZE));
            break;
        }
    }

    for (i = 0; i < MH_PLACE_TOBJ_COUNT; ++i) {
        uint8_t x, y, w, h;
        if (reader.pos + MH_PLACE_TOBJ_SIZE > len) {
            fprintf(stderr, "madhatter: Not enough data to read object text %zu\n", i);
            return -1;
        }
        x = mh_reader_read_u8(&reader);
        y = mh_reader_read_u8(&reader);
        w = mh_reader_read_u8(&reader);
        h = mh_reader_read_u8(&reader);
        mh_reader_seek(&reader, reader.pos + 6u);
        if (x == 0u && y == 0u && w == 0u && h == 0u) {
            mh_reader_seek(&reader, reader.pos + ((MH_PLACE_TOBJ_COUNT - i - 1u) * MH_PLACE_TOBJ_SIZE));
            break;
        }
    }

    place->bg_ani_count = 0u;
    for (i = 0; i < MH_PLACE_BGANI_COUNT; ++i) {
        mh_place_bg_ani bg_ani;
        if (reader.pos + MH_PLACE_BGANI_SIZE > len) {
            fprintf(stderr, "madhatter: Not enough data to read background animation %zu\n", i);
            return -1;
        }

        memset(&bg_ani, 0, sizeof(bg_ani));
        bg_ani.x = mh_reader_read_u8(&reader);
        bg_ani.y = mh_reader_read_u8(&reader);
        for (size_t j = 0; j < sizeof(bg_ani.name) - 1u; ++j) {
            bg_ani.name[j] = (char)mh_reader_read_u8(&reader);
        }
        bg_ani.name[sizeof(bg_ani.name) - 1u] = '\0';

        if (bg_ani.x == 0u && bg_ani.y == 0u && bg_ani.name[0] == '\0') {
            mh_reader_seek(&reader, reader.pos + ((MH_PLACE_BGANI_COUNT - i - 1u) * MH_PLACE_BGANI_SIZE));
            break;
        }

        place->bg_ani[place->bg_ani_count++] = bg_ani;
    }

    for (i = 0; i < MH_PLACE_EVENT_COUNT; ++i) {
        uint8_t x, y, w, h;
        if (reader.pos + MH_PLACE_EVENT_SIZE > len) {
            fprintf(stderr, "madhatter: Not enough data to read event %zu\n", i);
            return -1;
        }
        x = mh_reader_read_u8(&reader);
        y = mh_reader_read_u8(&reader);
        w = mh_reader_read_u8(&reader);
        h = mh_reader_read_u8(&reader);
        mh_reader_seek(&reader, reader.pos + 4u);
        if (x == 0u && y == 0u && w == 0u && h == 0u) {
            mh_reader_seek(&reader, reader.pos + ((MH_PLACE_EVENT_COUNT - i - 1u) * MH_PLACE_EVENT_SIZE));
            break;
        }
    }

    place->exit_count = 0;
    for (i = 0; i < MH_PLACE_MAX_EXITS; ++i) {
        mh_place_exit exit;
        size_t remaining;

        if (reader.pos + MH_PLACE_EXIT_SIZE > len) {
            fprintf(stderr, "madhatter: Not enough data to read exit %zu\n", i);
            return -1;
        }

        memset(&exit, 0, sizeof(exit));
        exit.bounding.x = mh_reader_read_u8(&reader);
        exit.bounding.y = mh_reader_read_u8(&reader);
        exit.bounding.width = mh_reader_read_u8(&reader);
        exit.bounding.height = mh_reader_read_u8(&reader);
        exit.id_image = mh_reader_read_u8(&reader);
        exit.mode_decoding = mh_reader_read_u8(&reader);
        mh_reader_seek(&reader, reader.pos + 1u); /* reserved byte */
        exit.id_sound = mh_reader_read_u8(&reader);
        exit.pos_transition_x = mh_reader_read_u8(&reader);
        exit.pos_transition_y = mh_reader_read_u8(&reader);
        exit.spawn_data = mh_reader_read_u16_le(&reader);

        if (mh_place_bounding_is_empty(&exit.bounding)) {
            remaining = MH_PLACE_MAX_EXITS - 1u - i;
            mh_reader_seek(&reader, reader.pos + (remaining * MH_PLACE_EXIT_SIZE));
            break;
        }

        place->exits[place->exit_count++] = exit;
    }

    mh_reader_seek(&reader, reader.pos + MH_PLACE_TAIL_PAD);
    if (reader.pos + 2u > len) {
        fprintf(stderr, "madhatter: Not enough data to read place tail pad\n");
        return -1;
    }
    place->id_sound = mh_reader_read_u16_le(&reader);

    return 0;
}
