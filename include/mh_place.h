#ifndef MH_PLACE_H
#define MH_PLACE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MH_PLACE_MAX_EXITS 12
#define MH_PLACE_BGANI_COUNT 12u
#define MH_PLACE_EVENT_COUNT 16u

typedef struct {
    uint8_t x, y, width, height;
} mh_bounding_box;

typedef struct {
    uint8_t x, y;
    char name[31];
} mh_place_bg_ani;

typedef struct {
    mh_bounding_box bounding;
    uint16_t id_image;
    uint16_t id_event;
} mh_place_event;

typedef struct {
    mh_bounding_box bounding;
    uint8_t id_image;
    uint8_t mode_decoding;
    uint8_t id_sound;
    uint8_t pos_transition_x, pos_transition_y;
    uint16_t spawn_data;
} mh_place_exit;

typedef struct {
    uint8_t id_name_place;
    uint8_t pos_map_x, pos_map_y;
    uint8_t bg_main_id;
    uint8_t bg_map_id;
    uint16_t id_sound;
    mh_place_bg_ani bg_ani[MH_PLACE_BGANI_COUNT];
    size_t bg_ani_count;
    mh_place_event events[MH_PLACE_EVENT_COUNT];
    size_t event_count;
    mh_place_exit exits[MH_PLACE_MAX_EXITS];
    size_t exit_count;
} mh_place_data;

int mh_place_exit_can_spawn_event(const mh_place_exit *exit);

int mh_place_load_nds(mh_place_data *place, const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
