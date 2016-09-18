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
#ifndef READING_H
#define READING_H

void tr_reading_initialize(void);
/* Use:
   - bpm_app
   - d/k/D/K/s/r/R
   - offset
*/
void tro_set_app_dis_offset(struct tr_object *obj);

/* Use:
   - ps
   - offset_app
 */
void tro_set_obj_hiding(struct tr_object *o, int i);

/* Use:
   - obj_h
   - offset_app
   - bpm_app
 */
void tro_set_app_dis_offset_same_bpm(struct tr_object *o);

/* Use:
   - bpm_app
   - offset_app (after same_bpm)
*/
void tro_set_line_coeff(struct tr_object *o);

/* Use:
   - line_coeff
   - offset_app (after same_bpm)
 */
void tro_set_mesh_base(struct tr_object *o);

/* Use:
   - line_coeff
   - offset_app (after same_bpm)
   - mesh_base
 */
void tro_set_mesh_remove_intersection(struct tr_object *o);

/* Use:
   - ps
   - mesh
   - d/k/D/K/s/r/R
   - offset_app (after same_bpm)
*/
void tro_set_seen(struct tr_object *o);

/* Must be done after seen
   Use:
   - mesh
 */
void tro_free_mesh(struct tr_object *o);

/* Must be done after seen
   Use:
   - obj_h
 */
void tro_free_obj_hiding(struct tr_object *o);

/* Use:
   - seen
*/
void tro_set_reading_star(struct tr_object *o);

// all
void trm_compute_reading (struct tr_map *map);

#endif
