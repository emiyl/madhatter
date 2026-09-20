#ifndef MH_DATAFILES_H
#define MH_DATAFILES_H

#include <stddef.h>

#include "mh_archive.h"
#include "mh_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *root;
    char language[8];
} mh_datafiles;

int mh_datafiles_init(mh_datafiles *df, const char *root, const char *language);
void mh_datafiles_free(mh_datafiles *df);

int mh_datafiles_exists(const mh_datafiles *df, const char *rel_path);
int mh_datafiles_get_data(const mh_datafiles *df, const char *rel_path, mh_buffer *out);
int mh_datafiles_get_pack(const mh_datafiles *df, const char *rel_path, mh_archive *out);
int mh_datafiles_get_packed_data(const mh_datafiles *df,
                                  const char *pack_rel_path,
                                  const char *filename,
                                  mh_buffer *out);

#ifdef __cplusplus
}
#endif

#endif
