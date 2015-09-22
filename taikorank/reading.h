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


#ifndef READING_H
#define READING_H

double tr_obj_coeff_superpos (struct tr_object * obj);
double hidding (struct tr_object * obj1, struct tr_object * obj2);
double speed_change (struct tr_object * obj1, struct tr_object * obj2);
double speed (double bpm_app);

void compute_reading_offset (struct tr_map * map);
void compute_reading_hidding (struct tr_map * map);
void compute_reading_superposed (struct tr_map * map);
void compute_reading_speed (struct tr_map * map);

void compute_reading_star (struct tr_map * map);

void compute_reading (struct tr_map * map);

#endif
