#ifndef OSUX_READ_H
#define OSUX_READ_H

#include <glib.h>
#include "osux/error.h"

G_BEGIN_DECLS

static inline void xfread(void *buf, size_t size, size_t nmemb, FILE *stream)
{
    size_t res = fread(buf, size, nmemb, stream);
    if (res < nmemb) {
        if (feof(stream))
            osux_error("Unexpected end-of-file\n");
        else
            osux_error("Error reading file\n");
        exit(EXIT_FAILURE);
    }
}

G_END_DECLS

#endif // OSUX_READ_H
