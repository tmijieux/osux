#ifndef STRING2_H
#define STRING2_H

#include <stdarg.h>

char *xasprintf(const char *format, ...);
char *xvasprintf(const char *format, va_list ap);

#endif //STRING2_H
