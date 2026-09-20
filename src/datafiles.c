#include "mh_datafiles.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "mh_asset.h"
#include "mh_layton_pack.h"
#include "mh_layton_pack2.h"
#include "mh_result.h"

int mh_datafiles_init(mh_datafiles *df, const char *root, const char *language) {
    size_t root_len;

    if (!df || !root) {
        return -1;
    }

    root_len = strlen(root);
    df->root = (char *)malloc(root_len + 1u);
    if (!df->root) {
        return -1;
    }
    memcpy(df->root, root, root_len + 1u);

    memset(df->language, 0, sizeof(df->language));
    if (language) {
        snprintf(df->language, sizeof(df->language), "%s", language);
    } else {
        snprintf(df->language, sizeof(df->language), "en");
    }

    return 0;
}

void mh_datafiles_free(mh_datafiles *df) {
    if (!df) {
        return;
    }
    free(df->root);
    df->root = NULL;
}

/* Joins df->root and rel_path (stripping any leading slash from rel_path). */
static int mh_datafiles_build_path(const mh_datafiles *df, const char *rel_path, char *out, size_t out_size) {
    if (rel_path[0] == '/') {
        rel_path += 1;
    }
    if (snprintf(out, out_size, "%s/%s", df->root, rel_path) >= (int)out_size) {
        return -1;
    }
    return 0;
}

static int mh_datafiles_read_raw(const char *path, uint8_t **out_data, size_t *out_len) {
    FILE *fp;
    long size;
    uint8_t *buffer;
    size_t bytes_read;

    fp = fopen(path, "rb");
    if (!fp) {
        return -1;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }
    size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return -1;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return -1;
    }

    buffer = (uint8_t *)malloc((size_t)size > 0 ? (size_t)size : 1u);
    if (!buffer) {
        fclose(fp);
        return -1;
    }

    bytes_read = fread(buffer, 1, (size_t)size, fp);
    fclose(fp);
    if (bytes_read != (size_t)size) {
        free(buffer);
        return -1;
    }

    *out_data = buffer;
    *out_len = bytes_read;
    return 0;
}

int mh_datafiles_exists(const mh_datafiles *df, const char *rel_path) {
    char path[4096];
    FILE *fp;

    if (!df || !rel_path) {
        return 0;
    }
    if (mh_datafiles_build_path(df, rel_path, path, sizeof(path)) != 0) {
        return 0;
    }

    fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }
    fclose(fp);
    return 1;
}

int mh_datafiles_get_data(const mh_datafiles *df, const char *rel_path, mh_buffer *out) {
    char path[4096];
    uint8_t *raw = NULL;
    size_t raw_len = 0;
    mh_asset asset;
    int result;

    if (!df || !rel_path || !out) {
        return RESULT_ERR_INVALID_ARGUMENT;
    }
    if (mh_datafiles_build_path(df, rel_path, path, sizeof(path)) != 0) {
        return RESULT_ERR_UNKNOWN;
    }
    if (mh_datafiles_read_raw(path, &raw, &raw_len) != 0) {
        return RESULT_ERR_NOT_FOUND;
    }

    if (mh_asset_init_from_bytes(&asset, raw, raw_len) != 0) {
        free(raw);
        return RESULT_ERR_INVALID_ARGUMENT;
    }
    
    free(raw);

    result = mh_asset_decompress(&asset, out);
    mh_asset_free(&asset);
    return result;
}

int mh_datafiles_get_pack(const mh_datafiles *df, const char *rel_path, mh_archive *out) {
    mh_buffer data;
    int result;

    if (!df || !rel_path || !out) {
        return -1;
    }

    mh_buffer_init(&data);
    if (mh_datafiles_get_data(df, rel_path, &data) != 0) {
        return -1;
    }

    bool isLaytonPack2 =
        (data.len >= 4u && (
            memcmp(data.data, "LPC2", 4u) == 0 ||
            memcmp(data.data, "PCK2", 4u) == 0
        )) ||
        (data.len >= 16u && memcmp(data.data + 12u, "PCK2", 4u) == 0);

    if (isLaytonPack2) {
        result = mh_archive_load_layton_pack2(out, data.data, data.len);
    } else {
        result = mh_archive_load_layton_pack(out, data.data, data.len, 1);
    }

    mh_buffer_free(&data);
    return result;
}

int mh_datafiles_get_packed_data(const mh_datafiles *df,
                                  const char *pack_rel_path,
                                  const char *filename,
                                  mh_buffer *out) {
    mh_archive archive;
    mh_archive_entry *entry;
    int result;

    if (!df || !pack_rel_path || !filename || !out) {
        fprintf(stderr, "madhatter: invalid arguments to mh_datafiles_get_packed_data\n");
        return -1;
    }

    if (mh_datafiles_get_pack(df, pack_rel_path, &archive) != 0) {
        fprintf(stderr, "madhatter: failed to get pack '%s'\n", pack_rel_path);
        return -1;
    }

    entry = mh_archive_get(&archive, filename);
    if (!entry) {
        mh_archive_free(&archive);
        fprintf(stderr, "madhatter: failed to find entry '%s' in pack '%s'\n", filename, pack_rel_path);
        return -1;
    }

    result = mh_asset_decompress(&entry->asset, out);
    mh_archive_free(&archive);
    return result;
}
