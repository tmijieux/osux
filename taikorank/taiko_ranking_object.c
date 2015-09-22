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

#include <math.h>

#include "struct.h"
#include "taiko_ranking_object.h"

//--------------------------------------------------

double mpb_to_bpm (double mpb)
{
  // work for 'mpb to bpm' and for 'bpm to mpb'
  return MSEC_IN_MINUTE / mpb;
}

//---------------------------------------------------

int equal (double x, double y)
{
  return fabs(1. - x / y) <= (EPSILON / 100.);
}

//---------------------------------------------------
//---------------------------------------------------
//---------------------------------------------------

int tro_is_big (struct tr_object * obj)
{
  return (obj->type == 'D' || obj->type == 'K' ||
	  obj->type == 'R');
}

//---------------------------------------------------

int tro_is_bonus (struct tr_object * obj)
{
  return (obj->type == 's' || tro_is_slider(obj));
}

//---------------------------------------------------

int tro_is_slider (struct tr_object * obj)
{
  return (obj->type == 'r' || obj->type == 'R');
}

//---------------------------------------------------

int tro_is_circle (struct tr_object * obj)
{
  return !tro_is_bonus (obj);
}

//---------------------------------------------------

int tro_is_kat (struct tr_object * obj)
{
  return (obj->type == 'k' || obj->type == 'K');
}

//---------------------------------------------------

int tro_is_don (struct tr_object * obj)
{
  return (obj->type == 'd' || obj->type == 'D');
}

//---------------------------------------------------
//---------------------------------------------------
//---------------------------------------------------

int tro_are_same_hand (struct tr_object * obj1,
		       struct tr_object * obj2)
{
  return ((obj1->l_hand == obj2->l_hand) ||
	  (obj1->r_hand == obj2->r_hand));
}

//---------------------------------------------------

int tro_are_same_type (struct tr_object * obj1,
		       struct tr_object * obj2)
{
  if (obj1->type == 's' || obj2->type == 's')
    return 1; // d and k are played
  if (obj1->type == 'r' || obj2->type == 'r')
    return 0; // suppose you play the easier...
  return ((tro_is_don(obj1) && tro_is_don(obj2)) ||
	  (tro_is_kat(obj1) && tro_is_kat(obj2)));
}

//---------------------------------------------------

int tro_are_same_density (struct tr_object * obj1,
			  struct tr_object * obj2)
{
  return (tro_are_same_type(obj1, obj2) &&
	  tro_are_same_hand(obj1, obj2));
}

//---------------------------------------------------
//---------------------------------------------------
//---------------------------------------------------
