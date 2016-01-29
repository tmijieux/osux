#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "string2.h"

#define HEURISTIC_SIZE 64

char *strdup(const char *s)
{
    char *d = calloc(strlen(s) + 1, 1);
    strcpy(d, s);
    return d;
}

int asprintf(void *hint, char **strp, const char *fmt, ...)
{
    va_list ap;

    char *buf = calloc(HEURISTIC_SIZE, 1);

    va_start(ap, fmt);
    int n = vsnprintf(buf, HEURISTIC_SIZE, fmt, ap);
    if (n >= HEURISTIC_SIZE) {
	buf = realloc(buf, n + 1);
	va_start(ap, fmt);	// important !!
	vsnprintf(buf, n + 1, fmt, ap);
    }

    *strp = buf;
    return n;
}

char *strstrip(const char *str)
{
    char *strip_ = strdup(str);
    size_t l = strlen(str);
    if (strip_[l - 1] == '\n')
	strip_[l - 1] = '\0';

    for (int i = 0; i < l - 1; ++i)
	if (strip_[i] == '\t')
	    strip_[i] = ' ';
    return strip_;
}

char *strstrip2(const char *str)
{
    char *strip_ = strdup(str);
    size_t l = strlen(str);
    if (strip_[l - 1] == '"')
	strip_[l - 1] = '\0';

    return strip_;
}
