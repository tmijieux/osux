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


#ifndef STRUCT_H
#define STRUCT_H

//----------------------------------------

struct tr_map
{
  // Name info
  char * title;
  char * creator;
  char * diff;
  
  // Song Info
  float od;

  // Taiko objects
  int nb_object;
  struct tr_object * object;

  // stars *-*
  double density_star;
  double reading_star;
  double pattern_star;
  double accuracy_star;
};

//----------------------------------------

struct tr_object
{
  // ---- basic data
  int offset;
  int end_offset;
  char type; // d D k K s(spinner) r(roll) R(roll)
  double bpm_app;

  // ---- additionnal data
  int l_hand;
  int r_hand;
  int rest; // printed in basic

  // ---- density data
  double density_raw;
  double density_color;

  // ---- reading data
  int offset_app;
  int end_offset_app;
  int offset_dis;
  int end_offset_dis;
  double superposed;   // percentage
  double hidden;       // how it is hidden by others
  double hide;         // how it hide others
  double speed;
  double speed_change;

  // ---- star
  double density_star;
  double reading_star;
  double pattern_star;
  double accuracy_star;
};

//----------------------------------------


#endif
