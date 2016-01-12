/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "split.h"

unsigned int string_split(const char *str, const char *delim, char ***buf_addr)
{
    if (NULL == str) {
        *buf_addr = NULL;
        return 0;
    }

    char *strw = strdup(str);
    struct list *li = list_new(0);
    char *saveptr;
    char *p =  strtok_r(strw, delim, &saveptr);
    while (p != NULL) {
	list_add(li, strdup(p));
	p = strtok_r(NULL, delim, &saveptr);
    }
    free(strw);
    
    unsigned int s = list_size(li);
    if (!s) {
	*buf_addr = NULL;
    } else {
	*buf_addr = malloc(sizeof(*buf_addr) * s);
        for (int i = 1; i <= s; ++i)
            (*buf_addr)[s - i] = list_get(li, i);
    }
    list_free(li);
    return s;
}
