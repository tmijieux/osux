/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
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

// offset app & dis
#define OFFSET_MIN 10000
#define OFFSET_MAX (-obj->obj_dis / obj->obj_app * OFFSET_MIN)

void trm_treatment_hand (struct tr_map * map);
void trm_treatment_rest (struct tr_map * map);
void trm_treatment_app_dis_offset (struct tr_map * map);
void trm_treatment_visible_time (struct tr_map * map);

void trm_treatment (struct tr_map * map);

#endif //TREATMENT_H
