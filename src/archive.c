#include "archive.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int mh_archive_ensure_capacity(mh_archive *archive, size_t required) {
    mh_archive_entry *new_entries = NULL;
    size_t new_capacity = 0u;

    if (!archive) {
        return -1;
    }

    if (archive->capacity >= required) {
        return 0;
    }

    new_capacity = archive->capacity == 0u ? 4u : archive->capacity;
    while (new_capacity < required) {
        new_capacity *= 2u;
    }

    new_entries = realloc(archive->entries, new_capacity * sizeof(*new_entries));
    if (!new_entries) {
        return -1;
    }

    archive->entries = new_entries;
    archive->capacity = new_capacity;
    return 0;
}

int mh_archive_init(mh_archive *archive) {
    if (!archive) {
        return -1;
    }
    memset(archive, 0, sizeof(*archive));
    return 0;
}

void mh_archive_free(mh_archive *archive) {
    size_t i = 0u;

    if (!archive) {
        return;
    }

    for (i = 0u; i < archive->count; ++i) {
        free(archive->entries[i].name);
        mh_asset_free(&archive->entries[i].asset);
    }

    free(archive->entries);
    archive->entries = NULL;
    archive->count = 0u;
    archive->capacity = 0u;
}

int mh_archive_add_file(mh_archive *archive, const char *name, const uint8_t *data, size_t len) {
    mh_archive_entry *entry = NULL;
    size_t name_len = 0u;

    if (!archive || !name || !data || len == 0u) {
        return -1;
    }

    if (mh_archive_ensure_capacity(archive, archive->count + 1u) != 0) {
        return -1;
    }

    entry = &archive->entries[archive->count];
    memset(entry, 0, sizeof(*entry));

    name_len = strlen(name) + 1u;
    entry->name = malloc(name_len);
    if (!entry->name) {
        return -1;
    }
    memcpy(entry->name, name, name_len);

    if (mh_asset_init_from_bytes(&entry->asset, data, len) != 0) {
        free(entry->name);
        entry->name = NULL;
        return -1;
    }

    archive->count += 1u;
    return 0;
}

mh_archive_entry *mh_archive_get(mh_archive *archive, const char *name) {
    size_t i = 0u;

    if (!archive || !name) {
        fprintf(stderr, "madhatter: invalid arguments to mh_archive_get\n");
        return NULL;
    }

    for (i = 0u; i < archive->count; ++i) {
        if (strcmp(archive->entries[i].name, name) == 0) {
            return &archive->entries[i];
        }
    }

    return NULL;
}
