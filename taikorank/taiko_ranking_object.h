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


#ifndef TRO_H
#define TRO_H

// percentage for equal
#define EPSILON 1.5

#define MSEC_IN_MINUTE 60000.

double mpb_to_bpm (double mpb);
int equal (double x, double y);

int tro_is_big (struct tr_object * obj);
int tro_is_bonus (struct tr_object * obj);
int tro_is_slider (struct tr_object * obj);
int tro_is_circle (struct tr_object * obj);
int tro_is_kat (struct tr_object * obj);
int tro_is_don (struct tr_object * obj);

int tro_are_same_hand (struct tr_object * obj1,
		       struct tr_object * obj2);
int tro_are_same_type (struct tr_object * obj1,
		       struct tr_object * obj2);
int tro_are_same_density (struct tr_object * obj1,
			  struct tr_object * obj2);


#endif
