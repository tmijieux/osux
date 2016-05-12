#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "string2.h"

#define HEURISTIC_SIZE 64

char *xasprintf(const char *fmt, ...)
{
    va_list ap;
    char *buf = NULL;

    int n = vsnprintf(buf, HEURISTIC_SIZE, fmt, ap);
    if (n >= HEURISTIC_SIZE) {
	buf = realloc(buf, n + 1);
	va_start(ap, fmt);	// important !!
	vsnprintf(buf, n + 1, fmt, ap);
    }

    return buf;
}
