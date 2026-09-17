#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nazo.h"

static void expect_eq_int(const char *label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "%s: got %d want %d\n", label, got, want);
        exit(1);
    }
}

static void expect_eq_str(const char *label, const char *got, const char *want) {
    if (!got || !want || strcmp(got, want) != 0) {
        fprintf(stderr, "%s: got '%s' want '%s'\n", label, got ? got : "(null)", want ? want : "(null)");
        exit(1);
    }
}

int main(void) {
    mh_nazo_data src = {0};
    mh_nazo_data loaded = {0};
    mh_buffer blob = {0};
    int rc;

    src.id_external = 1234u;
    src.id_tutorial = 7u;
    src.picarat_decay_stages[0] = 1u;
    src.picarat_decay_stages[1] = 2u;
    src.picarat_decay_stages[2] = 3u;
    src.use_luke_solver = 0;
    src.use_luke_voicelines = 1;
    src.has_answer_background = 1;
    src.use_language_prompt_background = 1;
    src.use_language_answer_background = 1;
    src.index_place = 9u;
    src.id_handler = 4u;
    src.bg_main_id = 5u;
    src.bg_sub_id = 6u;
    src.id_reward = 11;
    src.text_name = "Puzzle Name";
    src.text_prompt = "Prompt text";
    src.text_correct = "Correct text";
    src.text_incorrect = "Incorrect text";
    src.text_hint[0] = "Hint zero";
    src.text_hint[1] = "Hint one";
    src.text_hint[2] = "Hint two";

    rc = mh_nazo_save_nds(&src, &blob);
    if (rc != 0) {
        fprintf(stderr, "save_nds failed\n");
        return 1;
    }

    rc = mh_nazo_load_nds(&loaded, blob.data, blob.len);
    if (rc != 0) {
        fprintf(stderr, "load_nds failed\n");
        mh_buffer_free(&blob);
        return 1;
    }

    expect_eq_int("id_external", (int)loaded.id_external, 1234);
    expect_eq_int("id_tutorial", (int)loaded.id_tutorial, 7);
    expect_eq_int("picarat0", (int)loaded.picarat_decay_stages[0], 1);
    expect_eq_int("picarat1", (int)loaded.picarat_decay_stages[1], 2);
    expect_eq_int("picarat2", (int)loaded.picarat_decay_stages[2], 3);
    expect_eq_int("index_place", (int)loaded.index_place, 9);
    expect_eq_int("id_handler", (int)loaded.id_handler, 4);
    expect_eq_int("bg_main_id", (int)loaded.bg_main_id, 5);
    expect_eq_int("bg_sub_id", (int)loaded.bg_sub_id, 6);
    expect_eq_int("id_reward", (int)loaded.id_reward, 11);
    expect_eq_int("flag_luke_solver", (int)loaded.use_luke_solver, 0);
    expect_eq_int("flag_luke_voice", (int)loaded.use_luke_voicelines, 1);
    expect_eq_int("flag_answer_bg", (int)loaded.has_answer_background, 1);
    expect_eq_int("flag_lang_prompt", (int)loaded.use_language_prompt_background, 1);
    expect_eq_int("flag_lang_answer", (int)loaded.use_language_answer_background, 1);
    expect_eq_str("name", loaded.text_name, "Puzzle Name");
    expect_eq_str("prompt", loaded.text_prompt, "Prompt text");
    expect_eq_str("correct", loaded.text_correct, "Correct text");
    expect_eq_str("incorrect", loaded.text_incorrect, "Incorrect text");
    expect_eq_str("hint0", loaded.text_hint[0], "Hint zero");
    expect_eq_str("hint1", loaded.text_hint[1], "Hint one");
    expect_eq_str("hint2", loaded.text_hint[2], "Hint two");

    mh_nazo_free(&loaded);
    mh_buffer_free(&blob);
    puts("nazo roundtrip passed");
    return 0;
}
