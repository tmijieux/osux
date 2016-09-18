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
#ifndef TR_LINEAR_FUN_H
#define TR_LINEAR_FUN_H

struct vector;
struct linear_fun;

struct linear_fun * lf_new(struct vector * v);
void lf_free(struct linear_fun * lf);

// Values must be sorted
struct linear_fun * cst_lf(GHashTable * ht, const char * key);

double lf_eval(struct linear_fun * lf, double x);

void lf_print(struct linear_fun * lf);
void lf_dump(struct linear_fun * lf);

#endif // TR_LINEAR_FUN_H
