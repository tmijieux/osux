#ifndef OSUX_UTIL_H
#define OSUX_UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <glib.h>

#define HANDLE_ARRAY_SIZE(array, array_size, array_capacity)            \
    do {                                                                \
        if (array_size+1 > array_capacity) {                            \
            array_capacity *= 2;                                        \
            g_assert(array_capacity > array_size);                      \
            array = g_realloc(array, sizeof(*(array)) * array_capacity); \
        }                                                               \
    } while (0)                                                         \

#define ALLOC_ARRAY(array_var, size_var, size_literal)                  \
    do {                                                                \
        array_var = g_malloc((size_literal) * sizeof(*(array_var)));    \
        size_var = (size_literal);                                      \
    } while (0)


#define ARRAY_SIZE(array) (sizeof (array) / sizeof((array)[0]))

char *osux_getline(GIOChannel *chan);
char *bytearray2hexstr(uint8_t const *bytearray, size_t size);
char *osux_get_file_hashstr(char const *file_path);
unsigned strsplit_size(char **split);
GIOChannel *osux_open_text_file_reading(char const *file_path);

#endif // OSUX_UTIL_H
