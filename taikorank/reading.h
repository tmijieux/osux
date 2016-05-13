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

/* Use:
   - ps
   - offset_app
   - d/k/D/K/s/r/R
   - c_app
   - bpm_app   
*/
void tro_set_seen(struct tr_object * o, int i);

/* Use:
   - seen
*/
void tro_set_reading_star(struct tr_object * o);

// all
void trm_compute_reading (struct tr_map * map);

#endif
