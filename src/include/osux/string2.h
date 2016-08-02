#ifndef OSUX_STRING_H
#define OSUX_STRING_H

#include <stdarg.h>

char *xasprintf(const char *format, ...);
char *xvasprintf(const char *format, va_list ap);

#endif // OSUX_STRING_H
