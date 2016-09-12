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
#ifndef FREQ_COUNTER_H
#define FREQ_COUNTER_H

struct counter;
typedef double (*inherit_fun)(const void*, const void*);
typedef double (*where_fun)(const void*);

struct counter * cnt_new(void);
void cnt_free(struct counter * c);

void cnt_add(struct counter * c, const void * data, const char * key,
	     double val);

double cnt_get_total(const struct counter * c);
double cnt_get_total_inherit(const struct counter * c, inherit_fun inherit);

double cnt_get_nb(const struct counter * c, const char * key);
double cnt_get_nb_inherit(const struct counter * c,
			  const char * key, inherit_fun inherit);
double cnt_get_nb_where(const struct counter * c, where_fun where);

void cnt_print(const struct counter * c);
void cnt_print_inherit(const struct counter * c, inherit_fun inherit);

#endif //FREQ_COUNTER_H
