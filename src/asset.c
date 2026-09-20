#include "mh_asset.h"

#include <stdlib.h>
#include <string.h>

#include "mh_lz10.h"
#include "mh_rle.h"
#include "mh_huffman.h"

void mh_asset_init(mh_asset *asset) {
    if (!asset) {
        return;
    }
    memset(asset, 0, sizeof(*asset));
}

void mh_asset_free(mh_asset *asset) {
    if (!asset) {
        return;
    }
    free(asset->data);
    asset->data = NULL;
    asset->len = 0;
    asset->compression_type = 0;
}

static int mh_asset_probe_compression(const uint8_t *data, size_t len, int type) {
    mh_buffer decoded = {0};
    int status = -1;

    if (!data || len == 0u) {
        return -1;
    }

    switch (type) {
        case MH_COMP_LZ10:
            status = mh_lz10_decompress(data, len, &decoded);
            break;
        case MH_COMP_HUFFMAN_4:
            status = mh_huffman_decompress(data, len, &decoded, 1);
            break;
        case MH_COMP_HUFFMAN_8:
            status = mh_huffman_decompress(data, len, &decoded, 0);
            break;
        case MH_COMP_RLE:
            status = mh_rle_decompress(data, len, &decoded);
            break;
        default:
            return -1;
    }

    if (status != 0 || decoded.len == 0u) {
        mh_buffer_free(&decoded);
        return -1;
    }

    mh_buffer_free(&decoded);
    return 0;
}

int mh_asset_init_from_bytes(mh_asset *asset, const uint8_t *data, size_t len) {
    int type = MH_COMP_NONE;
    size_t offset = 0u;

    if (!asset || !data || len == 0) {
        return -1;
    }

    mh_asset_init(asset);

    asset->data = malloc(len);
    if (!asset->data) {
        return -1;
    }

    memcpy(asset->data, data, len);
    asset->len = len;

    type = mh_file_detect_compression_type(data, len, offset);
    if (type == MH_COMP_NONE && len >= 8u) {
        offset = 4u;
        type = mh_file_detect_compression_type(data, len, offset);
    }
    if (type != MH_COMP_NONE && mh_asset_probe_compression(data + offset, len - offset, type) != 0) {
        type = MH_COMP_NONE;
    }

    asset->compression_type = type;
    return 0;
}

int mh_asset_decompress(mh_asset *asset, mh_buffer *out) {
    uint8_t *payload = NULL;
    size_t payload_len = 0;
    size_t offset = 0u;
    int status = -1;

    if (!asset || !out) {
        return -1;
    }

    mh_buffer_init(out);

    if (asset->len == 0) {
        return 0;
    }

    if (asset->compression_type == MH_COMP_NONE) {
        mh_buffer_append(out, asset->data, asset->len);
        return 0;
    }

    if (asset->compression_type == MH_COMP_LZ10 || asset->compression_type == MH_COMP_RLE ||
        asset->compression_type == MH_COMP_HUFFMAN_4 || asset->compression_type == MH_COMP_HUFFMAN_8) {
        if (asset->len >= 4u && asset->data[0] == (uint8_t)asset->compression_type) {
            offset = 0u;
        } else if (asset->len >= 8u && asset->data[4] == (uint8_t)asset->compression_type) {
            offset = 4u;
        } else {
            offset = 0u;
        }
    }

    payload = asset->data + offset;
    payload_len = asset->len - offset;

    switch (asset->compression_type) {
        case MH_COMP_LZ10:
            status = mh_lz10_decompress(payload, payload_len, out);
            break;
        case MH_COMP_RLE:
            status = mh_rle_decompress(payload, payload_len, out);
            break;
        case MH_COMP_HUFFMAN_4:
            status = mh_huffman_decompress(payload, payload_len, out, 1);
            break;
        case MH_COMP_HUFFMAN_8:
            status = mh_huffman_decompress(payload, payload_len, out, 0);
            break;
        case MH_COMP_NONE:
            mh_buffer_append(out, asset->data, asset->len);
            status = 0;
            break;
        default:
            status = -1;
            break;
    }

    return status;
}
