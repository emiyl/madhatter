#ifndef MH_ANIM_H
#define MH_ANIM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *pixels;
    int width;
    int height;
} mh_anim_frame;

typedef struct {
    uint32_t keyframe_index;
    uint32_t duration_frames;
    int frame_index;
} mh_anim_keyframe;

typedef struct {
    char name[31];
    int first_frame_index;
    size_t keyframe_count;
    mh_anim_keyframe *keyframes;
} mh_anim_animation;

typedef struct {
    mh_anim_frame *frames;
    size_t frame_count;
    mh_anim_animation *animations;
    size_t animation_count;
} mh_anim_image;

int mh_anim_decode_arc(const uint8_t *data, size_t len, mh_anim_image *out);
void mh_anim_free(mh_anim_image *image);

const mh_anim_animation *mh_anim_get_animation_by_name(const mh_anim_image *image, const char *name);
const mh_anim_frame *mh_anim_get_frame_by_animation_name(const mh_anim_image *image, const char *name);
int mh_anim_get_animation_frame_index(const mh_anim_image *image, const char *name, uint64_t elapsed_ms);

#ifdef __cplusplus
}
#endif

#endif
