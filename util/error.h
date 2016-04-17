#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define __FILENAME__ (strrchr(__FILE__, '/') ? \
                      strrchr(__FILE__, '/') + 1 : __FILE__)

#define osux_error(format, ...)                                      \
    fprintf(stderr, "\e[31;1mERROR: %s\e[32m:\e[31;1m"               \
            "%d\e[32m|\e[31;1m%s:\e[0m " format, __FILENAME__ ,      \
            __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);

#define osux_malloc(size__) malloc(size__)
#define osux_free(ptr__) free(ptr__)


#ifdef DEBUG

#define osux_debug(format, ...)                                      \
    fprintf(stderr, "\e[31;1mDEBUG: %s\e[32m:\e[31;1m"               \
            "%d\e[32m|\e[31;1m%s:\e[0m " format, __FILENAME__ ,      \
            __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);
#else

#define osux_debug(format, ...)

#endif


#endif //ERROR_H
