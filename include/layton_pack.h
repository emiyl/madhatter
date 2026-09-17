#ifndef MH_LAYTON_PACK_H
#define MH_LAYTON_PACK_H

#include <stddef.h>
#include <stdint.h>

#include "archive.h"

#ifdef __cplusplus
extern "C" {
#endif

int mh_archive_load_layton_pack(mh_archive *archive, const uint8_t *data, size_t len, int version);
int mh_archive_save_layton_pack(const mh_archive *archive, mh_buffer *out, int version);

#ifdef __cplusplus
}
#endif

#endif
