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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

#include <visibility.h>

#include "regread.h"

static char *str = NULL;

__internal
const char *reg_read_match(const char *line, regmatch_t mtch[], int i)
{
    free(str);
    int size = mtch[i].rm_eo - mtch[i].rm_so;
    
    str = malloc( size + 1 );
    memcpy( str, line + mtch[i].rm_so, size );
    str [size] = 0;
    return str;
}

__internal
__attribute__((destructor))
void reg_free_str(void)
{
    free(str);
    str = NULL;
}
