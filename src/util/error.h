#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#define osux_error(format, ...)                                 \
    fprintf(stderr, "%s:%s:%s: " format, __FILE__ ,             \
            __PRETTY_FUNCTION__, __LINE__, __VA_ARGS__); 

#endif //ERROR_H
