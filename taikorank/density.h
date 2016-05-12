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

#ifndef DENSITY_H
#define DENSITY_H

// work independently
void tro_set_density_raw(struct tr_object * objs, int i);
void tro_set_density_color(struct tr_object * objs, int i);

// work independently, must be done after all other
void tro_set_density_star(struct tr_object * obj);

// all
void trm_compute_density(struct tr_map * map);

#endif
