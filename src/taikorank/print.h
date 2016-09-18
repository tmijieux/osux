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
#ifndef PRINT_H
#define PRINT_H

#define OUTPUT      stdout
#define OUTPUT_INFO stderr
#define OUTPUT_ERR  stderr

#define FILTER_BASIC        (1 << 0)
#define FILTER_BASIC_PLUS   (1 << 1)
#define FILTER_ADDITIONNAL  (1 << 2)
#define FILTER_DENSITY      (1 << 3)
#define FILTER_READING      (1 << 4)
#define FILTER_READING_PLUS (1 << 5)
#define FILTER_PATTERN      (1 << 6)
#define FILTER_ACCURACY     (1 << 7)
#define FILTER_STAR         (1 << 8)

enum print_level {
    NONE    = -1,
    ERROR   = 0,
    WARNING = 1,
    ALL     = 9
};

void tr_set_print_level(enum print_level level);

void tr_error(const char *s, ...);
void tr_warning(const char *s, ...);

void print_string_size(const char *s, int max, FILE *output);

#endif
