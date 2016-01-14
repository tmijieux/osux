#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"

FILE *osux_open_resource(const char *path, const char *mode)
{
    char *full_path = osux_prefix_path(PKG_DATA_DIR, path);
    FILE *f = fopen(full_path, mode);
    fprintf(stderr," DEBUG %s\n" , full_path);
    free(full_path);

    return f;
}

char *osux_prefix_path(const char *prefix, const char *path)
{
    size_t len = 1 + strlen(prefix) + strlen(path);
    char *prefixed_path = malloc(len);
    strcpy(prefixed_path, prefix);
    strcat(prefixed_path, path);
    return prefixed_path;
}
