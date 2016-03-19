/*
 *  Copyright (©) 2015-2016 Lucas Maugère, Thomas Mijieux
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "print.h"

#define TR_PREFIX "[Taiko Ranking] "

//-------------------------------------------------

void tr_error(const char * s, ...)
{
    va_list vl;
    va_start(vl, s);
    fprintf(OUTPUT_ERR, TR_PREFIX "\033[91mError: \033[0m");
    vfprintf(OUTPUT_ERR, s, vl);
    fprintf(OUTPUT_ERR, "\n");
}

void tr_warning(const char * s, ...)
{
    va_list vl;
    va_start(vl, s);
    fprintf(OUTPUT_ERR, TR_PREFIX "\033[93mWarning: \033[0m");
    vfprintf(OUTPUT_ERR, s, vl);
    fprintf(OUTPUT_ERR, "\n");
}

void print_string_size(const char *src, int max, FILE * output)
{
    char * s = strdup(src);
    int length = strlen(s);
    if (length >= max) {
	s[max-1] = '\0';
	s[max-2] = '.';
	s[max-3] = '.';
	s[max-4] = '.';
	length = max;
    }
    fprintf(output, "%s", s);
    length = max - length - 1;
    for (int i = 0; i < length; i++)
	fprintf(output, " ");
    fprintf(output, "\t");
    free(s);
}
