#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>


enum osux_error {
    OSUX_UNSPECIFIED_ERROR       = -1,
    OSUX_ERR_FILE_PERM           = -2,
    OSUX_ERR_BAD_OSU_VERSION     = -3,
    OSUX_ERR_MALFORMED_OSU_FILE  = -4,
    OSUX_ERR_INVALID_HITOBJECT   = -5,
    OSUX_ERR_DATABASE            = -6,
};

#ifndef __GNUC__
#	define __PRETTY_FUNCTION__    __FUNCDNAME__
#endif


#define __FILENAME__ (strrchr(__FILE__, '/') ?                  \
                      strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef _WIN32
#   define osux_error(format, ...)                              \
    fprintf(stderr, "ERROR: %s: %d|%s: " format, __FILENAME__ , \
            __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)
#else
#   define osux_error(format, ...)                              \
    fprintf(stderr, "\e[31;1mERROR: %s\e[32m:\e[31;1m"          \
            "%d\e[32m|\e[31;1m%s:\e[0m " format, __FILENAME__ , \
            __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)
#endif


#define osux_malloc(size__) malloc(size__)
#define osux_free(ptr__) free(ptr__)


#ifdef DEBUG

#define osux_debug(format, ...)                                 \
    fprintf(stderr, "\e[31;1mDEBUG: %s\e[32m:\e[31;1m"          \
            "%d\e[32m|\e[31;1m%s:\e[0m " format, __FILENAME__ , \
            __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);
#else

#define osux_debug(format, ...)

#endif


#endif //ERROR_H
