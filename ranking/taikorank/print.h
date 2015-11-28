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

//#define FILTER_APPLY       (FILTER_BASIC | FILTER_DENSITY)
//#define FILTER_APPLY       (FILTER_BASIC | FILTER_READING)
#define FILTER_APPLY       (FILTER_BASIC_PLUS | FILTER_PATTERN)

#define STR_MODS_LENGTH 3
#define MAX_MODS        4

#define DB_FILE_PATH "data.sql"

#define TABLE_USER      "tr_user"
#define TABLE_USER_NAME "name"

#define TABLE_BMS         "tr_beatmap_set"
#define TABLE_BMS_TITLE   "title"
#define TABLE_BMS_ARTIST  "artist"
#define TABLE_BMS_SOURCE  "source"
#define TABLE_BMS_USER_ID "user_ID"


void print_string_size(char *s, int max, FILE * output);

#endif
