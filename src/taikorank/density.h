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
#ifndef TR_DENSITY_H
#define TR_DENSITY_H

void tr_density_initialize(void);
/* Use:
   - ps
   - d/k/D/K/s/r/R
   - offset
   - length
*/
void tro_set_density_raw(struct tr_object *o, int i);

/* Use:
   - ps
   - d/k/D/K/s/r/R
   - hands
   - offset
   - length
*/
void tro_set_density_color(struct tr_object *o, int i);

/* Use:
   - ps
   - d/k/D/K/s/r/R
   - hands
   - offset
   - length
*/
void tro_set_density_ddkk(struct tr_object *o, int i);

/* Use:
   - ps
   - d/k/D/K/s/r/R
   - hands
   - offset
   - length
*/
void tro_set_density_kddk(struct tr_object *o, int i);

/* Use:
   - density_raw
   - density_color
*/
void tro_set_density_star(struct tr_object *obj);

// all
void trm_compute_density(struct tr_map *map);

#endif // TR_DENSITY_H
