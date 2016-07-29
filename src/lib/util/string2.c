#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "osux/string2.h"

#define HEURISTIC_SIZE 64

char *xvasprintf(const char *format, va_list ap)
{
    va_list cpy;
    char *buf = malloc(sizeof(*buf) * HEURISTIC_SIZE);
    va_copy(cpy, ap);

    int n = vsnprintf(buf, HEURISTIC_SIZE, format, ap);
    if (n >= HEURISTIC_SIZE) {
	buf = realloc(buf, n + 1);
	vsnprintf(buf, n + 1, format, cpy);
    }
    va_end(cpy);
    return buf;
}

char *xasprintf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    char *res = xvasprintf(format, ap);
    va_end(ap);
    return res;
}
