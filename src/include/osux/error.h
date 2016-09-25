#ifndef OSUX_ERROR_H
#define OSUX_ERROR_H

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

G_BEGIN_DECLS

#define OSUX_ERROR_LIST(ERROR)                          \
    ERROR(OSUX_SUCCESS)                                 \
    ERROR(OSUX_ERR_UNKNOWN_ERROR)                       \
    ERROR(OSUX_ERR_INVAL)                               \
    ERROR(OSUX_ERR_FILE_ERROR)                          \
    ERROR(OSUX_ERR_FILE_BAD_ENCODING)                   \
    ERROR(OSUX_ERR_FILE_ACCESS)                         \
    ERROR(OSUX_ERR_BAD_OSU_VERSION)                     \
    ERROR(OSUX_ERR_MALFORMED_OSU_FILE)                  \
    ERROR(OSUX_ERR_DATABASE)                            \
    ERROR(OSUX_ERR_INVALID_COLOR)                       \
    ERROR(OSUX_ERR_INVALID_COLOR_TYPE)                  \
    ERROR(OSUX_ERR_INVALID_TIMINGPOINT)                 \
    ERROR(OSUX_ERR_INVALID_INHERITED_TIMINGPOINT)       \
    ERROR(OSUX_ERR_INVALID_HITOBJECT)                   \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_TYPE)              \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_SPINNER)           \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_HOLD)              \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_ADDON_HITSOUND)    \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_EDGE_SAMPLE)       \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_EDGE_SAMPLE_TYPE)  \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_SLIDER_TYPE)       \
    ERROR(OSUX_ERR_INVALID_HITOBJECT_SLIDER_POINTS)     \
    ERROR(OSUX_ERR_INVALID_EVENT)                       \
    ERROR(OSUX_ERR_INVALID_EVENT_OBJECT)                \
    ERROR(OSUX_ERR_INVALID_EVENT_COMMAND)               \
    ERROR(OSUX_ERR_INVALID_EVENT_LOOP_COMMAND)          \
    ERROR(OSUX_ERR_INVALID_EVENT_PARAMETER_COMMAND)     \
    ERROR(OSUX_ERR_INVALID_EVENT_TRIGGER_COMMAND)       \
    ERROR(OSUX_ERR_MEMORY_TOO_MUCH_NESTED_EVENT)        \
    ERROR(OSUX_ERR_REPLAY_LIFE_BAR)                     \
    ERROR(OSUX_ERR_REPLAY_DATA)                         \
    ERROR(OSUX_ERR_BUFFER_READER_RANGE)                 \
    ERROR(OSUX_ERR_AUTOCONVERT_NOT_SUPPORTED)           \
    ERROR(OSUX_ERR_INVALID_GAME_MODE)                   \
    ERROR(OSUX_ERR_GAME_MODE_NOT_SUPPORTED)             \


#define OSUX_ERROR_TO_ENUM(error) error,

enum osux_error_code {
    OSUX_ERROR_LIST(OSUX_ERROR_TO_ENUM)
};

char const *osux_errmsg(int errcode);

#ifndef __GNUC__
#define __PRETTY_FUNCTION__    __FUNCDNAME__
#endif


#define __FILENAME__ (strrchr(__FILE__, '/') ?                  \
                      strrchr(__FILE__, '/') + 1 : __FILE__)

#define osux_error(format_, ...)                                \
    do {                                                        \
        fprintf(stderr, "ERROR: %s:%d|%s: ",  __FILENAME__ ,    \
                __LINE__, __PRETTY_FUNCTION__);                 \
        fprintf(stderr, (format_), ##__VA_ARGS__);              \
    } while(0)

#define osux_fatal(format_, ...)                                        \
    do {                                                                \
        fprintf(stderr, "FATAL ERROR: %s:%d|%s: ",  __FILENAME__ ,      \
                __LINE__, __PRETTY_FUNCTION__);                         \
        fprintf(stderr, (format_), ##__VA_ARGS__);                      \
        osux_error(format_, ##__VA_ARGS__);                             \
        exit(EXIT_FAILURE);                                             \
    } while(0)

#define osux_warning(format_, ...)                              \
    do {                                                        \
        fprintf(stderr, "WARNING: %s:%d|%s: ",  __FILENAME__ ,  \
                __LINE__, __PRETTY_FUNCTION__);                 \
        fprintf(stderr, (format_), ##__VA_ARGS__);              \
    } while(0)


#define osux_malloc(size_) g_malloc0(size_)
#define osux_free(ptr_) g_free(ptr_)


#ifdef DEBUG
#define osux_debug(format_, ...)                                        \
    do {                                                                \
        fprintf(stderr, "DEBUG: %s:%d|%s: " format_, __FILENAME__ ,     \
                __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);          \
    } while(0)

#else // DEBUG
#define osux_debug(format_, ...) ((void) (format_))
#endif // DEBUG

G_END_DECLS

#endif // OSUX_ERROR_H
