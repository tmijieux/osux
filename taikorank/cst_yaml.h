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
#ifndef CST_YAML_H
#define CST_YAML_H

struct yaml_wrap * cst_get_yw(const char * file_name);

struct hash_table * yw_extract_ht(struct yaml_wrap * yw);
struct osux_list * yw_extract_list(struct yaml_wrap * yw);
char * yw_extract_scalar(struct yaml_wrap * yw);

struct osux_list * cst_list(struct hash_table * ht, const char * key);
double cst_f(struct hash_table * ht, const char * key);
int cst_i(struct hash_table * ht, const char * key);
char * cst_str(struct hash_table * ht, const char * key);
struct stats * cst_stats(struct hash_table * ht, const char * key);

#endif //CST_YAML_H
