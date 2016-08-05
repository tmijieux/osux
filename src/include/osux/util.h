#ifndef OSUX_UTIL_H
#define OSUX_UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define HANDLE_ARRAY_SIZE(array, array_size, array_capacity)            \
    do {                                                                \
        if (array_size+1 > array_capacity) {                            \
            array_capacity *= 2;                                        \
            g_assert(array_capacity > array_size);                      \
            array = g_realloc(array, sizeof(*(array)) * array_capacity); \
            memset(array+(array_capacity/2), 0,                         \
                   (array_capacity/2)*sizeof(*(array)));                \
        }                                                               \
    } while (0)                                                         \

char *osux_getline(FILE *file);
char *bytearray2hexstr(uint8_t const *bytearray, size_t size);
char *osux_get_file_hashstr(char const *file_path);
unsigned strsplit_size(char **split);

#endif // OSUX_UTIL_H
