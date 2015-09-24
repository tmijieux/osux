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

#ifndef PRINT_H
#define PRINT_H

#define OUTPUT     stdout
#define OUTPUT_ERR stderr

#define FILTER_BASIC        (1 << 0)
#define FILTER_BASIC_PLUS   (1 << 1)
#define FILTER_ADDITIONNAL  (1 << 2)
#define FILTER_DENSITY      (1 << 3)
#define FILTER_READING      (1 << 4)
#define FILTER_READING_PLUS (1 << 5)
#define FILTER_PATTERN      (1 << 6)
#define FILTER_ACCURACY     (1 << 7)
#define FILTER_STAR         (1 << 8)

#define FILTER_APPLY       (FILTER_BASIC | FILTER_READING)

#define STR_MODS_LENGTH 3
#define MAX_MODS        4

void print_all_tr_object (struct tr_map * map, int filter);
void print_one_tr_object (struct tr_object * obj, int filter);

void print_map_star (struct tr_map * map);

int print_one_mod (struct tr_map * map, int mods, int * i,
		   char * buffer, char * string);
void print_mods (struct tr_map * map);
void print_string_size (char * s, int max);
void print_map_final (struct tr_map * map);


#endif
