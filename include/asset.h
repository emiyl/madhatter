#ifndef MH_ASSET_H
#define MH_ASSET_H

#include <stddef.h>
#include <stdint.h>

#include "file.h"
#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t *data;
    size_t len;
    int compression_type;
} mh_asset;

void mh_asset_init(mh_asset *asset);
void mh_asset_free(mh_asset *asset);
int mh_asset_init_from_bytes(mh_asset *asset, const uint8_t *data, size_t len);
int mh_asset_decompress(mh_asset *asset, mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
