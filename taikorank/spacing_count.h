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
#ifndef SPACING_COUNT_H
#define SPACING_COUNT_H

struct list;

struct spacing {
  int rest;
  double nb;
};

struct list * spc_new(void);
void spc_free(struct list * spc);

void spc_add_f(struct list * spc, int rest, double val);
void spc_add(struct list * spc, int rest);
void spc_increase_f(struct list * spc, int rest, double val);
void spc_increase(struct list * spc, int rest);

void spc_print(struct list * spc);

#endif //SPACING_COUNT_H
