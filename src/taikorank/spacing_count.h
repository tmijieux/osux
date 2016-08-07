#ifndef TR_SPACING_COUNT_H
#define TR_SPACING_COUNT_H

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

struct spacing_count;

struct spacing_count * spc_new(int (*eq)(int, int));
void spc_free(struct spacing_count * spc);
void spc_add(struct spacing_count * spc, int rest, double val);
void spc_print(const struct spacing_count * spc);
double spc_get_total(const struct spacing_count * spc);
double spc_get_nb(const struct spacing_count * spc, int rest);

#endif // TR_SPACING_COUNT_H
