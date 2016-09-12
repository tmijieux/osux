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

#ifndef TREATMENT_H
#define TREATMENT_H

/* Use:
  - offset
 */
void tro_set_length(struct tr_object * obj);

/* Use:
   - ps
   - d/k/D/K/s/r/R
*/
void trm_set_hand(struct tr_map * map);

/* Use:
   - ps
   - offset
*/
void trm_set_rest(struct tr_map * map);

/* Use:
   - ps
*/
void trm_set_combo(struct tr_map * map);

// all
void trm_treatment(struct tr_map * map);

#endif //TREATMENT_H
