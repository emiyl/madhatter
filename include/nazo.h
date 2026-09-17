#ifndef MH_NAZO_H
#define MH_NAZO_H

#include <stddef.h>
#include <stdint.h>

#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t id_external;
    uint16_t length_header;
    uint8_t id_tutorial;
    uint8_t picarat_decay_stages[3];
    uint8_t flags;
    uint8_t use_luke_solver;
    uint8_t use_luke_voicelines;
    uint8_t has_answer_background;
    uint8_t use_language_prompt_background;
    uint8_t use_language_answer_background;
    uint8_t index_place;
    uint8_t id_handler;
    uint8_t bg_main_id;
    uint8_t reserved[2];
    uint8_t bg_sub_id;
    int8_t id_reward;
    char *text_name;
    char *text_prompt;
    char *text_correct;
    char *text_incorrect;
    char *text_hint[3];
} mh_nazo_data;

void mh_nazo_init(mh_nazo_data *nazo);
void mh_nazo_free(mh_nazo_data *nazo);
int mh_nazo_load_nds(mh_nazo_data *nazo, const uint8_t *data, size_t len);
int mh_nazo_load_hd(mh_nazo_data *nazo, const uint8_t *data, size_t len);
int mh_nazo_save_nds(const mh_nazo_data *nazo, mh_buffer *out);
int mh_nazo_save_hd(const mh_nazo_data *nazo, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
