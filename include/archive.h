#ifndef MH_ARCHIVE_H
#define MH_ARCHIVE_H

#include <stddef.h>
#include <stdint.h>

#include "asset.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *name;
    mh_asset asset;
} mh_archive_entry;

typedef struct {
    mh_archive_entry *entries;
    size_t count;
    size_t capacity;
} mh_archive;

int mh_archive_init(mh_archive *archive);
void mh_archive_free(mh_archive *archive);
int mh_archive_add_file(mh_archive *archive, const char *name, const uint8_t *data, size_t len);
mh_archive_entry *mh_archive_get(mh_archive *archive, const char *name);

#ifdef __cplusplus
}
#endif

#endif
