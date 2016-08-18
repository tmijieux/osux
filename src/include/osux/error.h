#ifndef OSUX_ERROR_H
#define OSUX_ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#define OSUX_ERROR_TO_ENUM(error) error,

#define OSUX_ERROR_LIST(ERROR)                                  \
    ERROR(OSUX_SUCCESS)                                         \
    ERROR(OSUX_ERR_UNKNOWN_ERROR)                               \
    ERROR(OSUX_ERR_INVAL)                                       \
    ERROR(OSUX_ERR_FILE_ERROR)                                  \
    ERROR(OSUX_ERR_FILE_ACCESS)                                 \
    ERROR(OSUX_ERR_BAD_OSU_VERSION)                             \
    ERROR(OSUX_ERR_MALFORMED_OSU_FILE)                          \
    ERROR(OSUX_ERR_DATABASE)                                    \
    ERROR(OSUX_ERR_INVALID_COLOR)                               \
    ERROR(OSUX_ERR_INVALID_TIMINGPOINT)                         \
    ERROR(OSUX_ERR_INVALID_INHERITED_TIMINGPOINT)               \
    ERROR(OSUX_ERR_INVALID_HITOBJECT)                           \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_TYPE)                      \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_SPINNER)                   \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_HOLD)                      \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_ADDON_HITSOUND)            \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_EDGE_SAMPLE)               \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_EDGE_SAMPLE_TYPE)          \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_SLIDER_TYPE)               \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_SLIDER_POINTS)             \
    ERROR(OSUX_ERR_INVALID_EVENT)                               \
    ERROR(OSUX_ERR_INVALID_EVENT_OBJECT)                        \
    ERROR(OSUX_ERR_INVALID_EVENT_COMMAND)                       \
    ERROR(OSUX_ERR_MEMORY_TOO_MUCH_NESTED_EVENT)                \
    ERROR(OSUX_ERR_REPLAY_LIFE_BAR)                             \
    ERROR(OSUX_ERR_REPLAY_DATA)                                 \
    ERROR(OSUX_ERR_BUFFER_READER_RANGE)                         \
    ERROR(OSUX_ERR_AUTOCONVERT_NOT_SUPPORTED)                   \
    ERROR(OSUX_ERR_INVALID_GAME_MODE)                           \
    ERROR(OSUX_ERR_GAME_MODE_NOT_SUPPORTED)                     \


#define CHECK_ERROR(expr, finally)              \
    do {                                        \
        int err_macro_;                         \
        if ((err_macro_ = (expr)) < 0) {        \
            finally;                            \
            return err_macro_;                  \
        }                                       \
    } while (0)

enum osux_error_code {
    OSUX_ERROR_LIST(OSUX_ERROR_TO_ENUM)
};

const char *osux_errmsg(int errcode);

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
#else // DEBUG
# define osux_debug(format, ...) ((void) (format))
#endif // DEBUG

#endif // OSUX_ERROR_H
