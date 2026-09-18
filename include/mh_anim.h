#ifndef MH_ANIM_H
#define MH_ANIM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* One decoded sub-image (RGBA8888, palette index 0 treated as transparent). */
typedef struct {
    uint8_t *pixels;
    int width;
    int height;
} mh_anim_frame;

typedef struct {
    char name[31];
    int first_frame_index; /* index into mh_anim_image.frames, or -1 if empty */
} mh_anim_animation;

/* C port of widebrim's AnimatedImage.fromBytesArc (non-arj tile-atlas sprite
 * format used for exit/button/UI graphics such as map/exit_%i.arc). Only the
 * static single-atlas subset needed to look up a frame by animation name is
 * implemented; keyframe timing/sub-animations are not. */
typedef struct {
    mh_anim_frame *frames;
    size_t frame_count;
    mh_anim_animation *animations;
    size_t animation_count;
} mh_anim_image;

int mh_anim_decode_arc(const uint8_t *data, size_t len, mh_anim_image *out);
void mh_anim_free(mh_anim_image *image);

/* Returns the frame referenced by the first keyframe of the named animation. */
const mh_anim_frame *mh_anim_get_frame_by_animation_name(const mh_anim_image *image, const char *name);

#ifdef __cplusplus
}
#endif

#endif
