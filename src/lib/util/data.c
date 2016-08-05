#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "osux/data.h"
#include "osux/error.h"

static FILE *
osux_open_prefixed(const char *prefix, const char *path, const char *mode)
{
    char *full_path = osux_prefix_path(prefix, path);
    FILE *f = g_fopen(full_path, mode);
    free(full_path);

    return f;
}

FILE *osux_open_resource(const char *path, const char *mode)
{
    return osux_open_prefixed(PKG_DATA_DIR, path, mode);
}

FILE *osux_open_config(const char *path, const char *mode)
{
    return osux_open_prefixed(PKG_CONFIG_DIR, path, mode);
}

char *osux_prefix_path(const char *prefix, const char *path)
{
    size_t len = 1 + strlen(prefix) + strlen(path);
    char *prefixed_path = malloc(len);
    strcpy(prefixed_path, prefix);
    strcat(prefixed_path, path);
    return prefixed_path;
}

static const char *song_path = "/mnt/windata/Songs2";

void osux_set_song_path(const char *path)
{
    song_path = path;
}

const char *osux_get_song_path(void)
{
    return song_path;
}
