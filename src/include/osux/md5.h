#ifndef OSUX_MD5_H
#define OSUX_MD5_H

#include <stdlib.h>
#include <stdint.h>
#include <openssl/md5.h>
#include <glib.h>

G_BEGIN_DECLS

typedef struct osux_md5_ {
    MD5_CTX ctx;
    uint8_t digest[MD5_DIGEST_LENGTH];
} osux_md5;

int osux_md5_init(osux_md5 *md5);
void osux_md5_update(osux_md5 *md5, void *data, size_t size);
int osux_md5_update_file(osux_md5 *md5, char const *file_path);
void osux_md5_update_string(osux_md5 *md5, char const *str);

void osux_md5_finalize(osux_md5 *md5);
const uint8_t *osux_md5_get_digest(osux_md5 const *md5);
size_t osux_md5_digest_length(osux_md5 const *md5);
#define osux_md5_free(x) //nathing

G_END_DECLS

#endif // OSUX_MD5_H
