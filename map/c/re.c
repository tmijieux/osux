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
#include <regex.h>

int re_match(const char *re, const char *str, int match_count, char **matches)
{
    int ret = -1;
    regex_t preg;
    regmatch_t *ma = NULL;
    
    if (match_count < 0) {
	return -1;
    }
	
    ret = regcomp(&preg, re, REG_EXTENDED | REG_NEWLINE);
    if (!ret)
	return -1;

    if (match_count > 0) {
	ma = malloc(sizeof(*ma) * (match_count+1));
    }
    ret = regexec(&preg, str, match_count+1, ma, 0);
    if (ret ==  REG_NOMATCH) {
	free(ma);
	return -1;
    }

    for (int i = 0; i < match_count; ++i) {
	int size = ma[i+1].rm_eo - ma[i+1].rm_so;
	matches[i] = malloc(size + 1);
	memcpy(matches[i], str + ma[i+1].rm_so, size);
	matches[size] = 0;
    }

    free(ma);
    regfree(&preg);
    return ret;
}

