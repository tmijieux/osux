#ifndef READ_H
#define READ_H

#include "util/error.h"

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

#endif //READ_H
