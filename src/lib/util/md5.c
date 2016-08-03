#include <glib.h>
#include "osux/md5.h"
#include "osux/error.h"


int osux_md5_init(osux_md5 *md5)
{
    MD5_Init(&md5->ctx);
    memset(md5->digest, 0, sizeof md5->digest);
}

int osux_md5_update_file(osux_md5 *md5, char const *file_path)
{
    gchar *data;
    gsize length;
    
    if (!g_file_get_contents(file_path, &data, &length, NULL))
        return OSUX_UNSPECIFIED_ERROR;
    osux_md5_update(md5, data, length);
    return 0;
}

void osux_md5_update_string(osux_md5 *md5, char const *str)
{
    MD5_Update(&md5->ctx, (uint8_t*) str, strlen(str));
}

void osux_md5_update(osux_md5 *md5, void *data, size_t size)
{
    MD5_Update(&md5->ctx, data, size);
}

void osux_md5_finalize(osux_md5 *md5)
{
    MD5_Final(md5->digest, &md5->ctx);
}

uint8_t const *osux_md5_get_digest(osux_md5 const *md5)
{
    return md5->digest;
}

size_t osux_md5_digest_length(osux_md5 const *md5)
{
    return sizeof md5->digest;
}

