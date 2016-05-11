/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
n *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "split.h"
#include "util/uleb128.h"
#include "util/read.h"

unsigned int string_split(const char *str, const char *delim, char ***buf_addr)
{
    if (NULL == str) {
        *buf_addr = NULL;
        return 0;
    }

    char *strw = strdup(str);
    struct osux_list *li = osux_list_new(0);
    char *saveptr;
    char *p =  strtok_r(strw, delim, &saveptr);
    while (p != NULL) {
	osux_list_add(li, strdup(p));
	p = strtok_r(NULL, delim, &saveptr);
    }
    free(strw);

    unsigned int s = osux_list_size(li);
    if (!s) {
	*buf_addr = NULL;
    } else {
	*buf_addr = malloc(sizeof(*buf_addr) * s);
        for (unsigned i = 1; i <= s; ++i)
            (*buf_addr)[s - i] = osux_list_get(li, i);
    }
    osux_list_free(li);
    return s;
}

int string_have_extension(const char *filename, const char *extension)
{
    size_t l = strlen(filename);
    size_t e = strlen(extension);
    if (l < e)
        return 0;
    return (strcmp(extension, filename + l - e) == 0);
}

void read_string_ULEB128(char **buf, FILE *f)
{
    uint8_t h;
    xfread(&h, 1, 1, f);
    if (h == 0x0B) {
	uint64_t s = read_ULEB128(f);
	*buf = malloc(s+1);
	xfread(*buf, s, 1, f);
	(*buf)[s] = 0;
    } else {
	*buf = NULL;
    }
}

void write_string_ULEB128(char *buf, FILE *f)
{
    uint8_t h = 0;
    size_t len = strlen(buf);
    if (0 == len) {
        fwrite(&h, 1, 1, f);
        return;
    } else {
        h = 0x0B;
        fwrite(&h, 1, 1, f);
        write_ULEB128(len, f, 0);
        fwrite(buf, 1, len, f);
    }
}
