#ifndef OSUX_STRING_H
#define OSUX_STRING_H

#include <stdarg.h>
#include <stdbool.h>

char *xasprintf(const char *format, ...);
char *xvasprintf(const char *format, va_list ap);
bool string_contains(char const *str, char c);

#endif // OSUX_STRING_H
