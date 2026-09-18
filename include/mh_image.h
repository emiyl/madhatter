#ifndef MH_IMAGE_H
#define MH_IMAGE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int mh_image_decode_static_arc(const uint8_t *data,
                                size_t len,
                                uint8_t **out_pixels,
                                int *out_width,
                                int *out_height);

#ifdef __cplusplus
}
#endif

#endif
