#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define osux_error(format, ...)                                 \
    fprintf(stderr, "\e[31;1mERROR: %s\e[32m:\e[31;1m"          \
            "%d\e[32m|\e[31;1m%s:\e[0m " format, __FILE__ ,     \
            __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);

#define osux_malloc(size__) malloc(size__)
#define osux_free(ptr__) free(ptr__)

#endif //ERROR_H
