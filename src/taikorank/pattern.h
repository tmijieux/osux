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

#ifndef PATTERN_H
#define PATTERN_H

void tr_pattern_initialize(void);

/* Use:
   - d/k/D/K/s/r/R
   - offset
*/
void tro_set_pattern_proba(struct tr_object * o, int i);

/* Use:
   - ps
   - d/k/D/K/s/r/R
*/
void tro_set_type(struct tr_object * o);

/* Use:
   - proba
   - type
*/
void tro_set_patterns(struct tr_object * o, int i, int nb);

/* Use:
   - patterns
   - offset
*/
void tro_set_pattern_freq(struct tr_object * o, int i);

/* Use:
   - pattern_freq
*/
void tro_set_pattern_star(struct tr_object * o);

/* Must be done after pattern_freq
   Use:
   - patterns
*/
void tro_free_patterns(struct tr_object * o);

// all
void trm_compute_pattern (struct tr_map * map);

#endif //PATTERN_H
