#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "osux/string.h"

bool string_contains(char const *str, char c)
{
    return strchr(str, c) != NULL;
}
