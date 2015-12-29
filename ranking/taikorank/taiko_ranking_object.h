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

//---------------------------------------------------------------

enum played_state
{
  GOOD, BAD, MISS, BONUS
};

struct tr_object
{
  // ---- basic data
  int offset;
  int end_offset;     // printed in basic+
  char type;          // d D k K s(spinner) r(roll) R(roll)
  double bpm_app;
  
  enum played_state ps;

  // ---- additionnal data
  int l_hand;          
  int r_hand;          
  int rest;            // printed in basic
  double obj_app;      // nb obj displayable on screen, without overlapping
  double obj_dis;      // nb obj where disappear
  int offset_app;      // printed in reading
  int end_offset_app;  // printed in reading+
  int offset_dis;      // printed in reading
  int end_offset_dis;  // printed in reading+
  int visible_time;    // printed in reading 
  int invisible_time;  // printed in reading

  // ---- pattern data
  double proba;
  double * alt;
  
  // ---- density data
  double density_raw;
  double density_color;

  // ---- reading data
  double superposed;   // percentage
  double hidden;       // how it is hidden by others
  double hide;         // how it hide others
  double slow;         // slow, hard to acc
  double fast;         // fast, hard to see
  double speed_change;

  // ---- star
  double density_star;
  double reading_star;
  double pattern_star;
  double accuracy_star;
  double final_star;
};

//-------------------------------------------------------------

void tro_print(struct tr_object * obj, int filter);

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
