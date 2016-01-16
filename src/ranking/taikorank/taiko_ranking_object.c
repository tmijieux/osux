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

#include <stdio.h>
#include <math.h>

#include "print.h"

#include "taiko_ranking_object.h"

//--------------------------------------------------

double mpb_to_bpm(double mpb)
{
  // work for 'mpb to bpm' and for 'bpm to mpb'
  return MSEC_IN_MINUTE / mpb;
}

//---------------------------------------------------

int equal(double x, double y)
{
  return fabs(1. - x / y) <= (EPSILON / 100.);
}

//---------------------------------------------------
//---------------------------------------------------
//---------------------------------------------------

int tro_is_big(struct tr_object * obj)
{
  return (obj->type == 'D' || obj->type == 'K' ||
	  obj->type == 'R');
}

//---------------------------------------------------

int tro_is_bonus(struct tr_object * obj)
{
  return (obj->type == 's' || tro_is_slider(obj));
}

//---------------------------------------------------

int tro_is_slider(struct tr_object * obj)
{
  return (obj->type == 'r' || obj->type == 'R');
}

//---------------------------------------------------

int tro_is_circle(struct tr_object * obj)
{
  return !tro_is_bonus(obj);
}

//---------------------------------------------------

int tro_is_kat(struct tr_object * obj)
{
  return (obj->type == 'k' || obj->type == 'K');
}

//---------------------------------------------------

int tro_is_don(struct tr_object * obj)
{
  return (obj->type == 'd' || obj->type == 'D');
}

//---------------------------------------------------
//---------------------------------------------------
//---------------------------------------------------

int tro_are_same_hand(struct tr_object * obj1,
		      struct tr_object * obj2)
{
  return ((obj1->l_hand == obj2->l_hand) ||
	  (obj1->r_hand == obj2->r_hand));
}

//---------------------------------------------------

int tro_are_same_type(struct tr_object * obj1,
		      struct tr_object * obj2)
{
  if(obj1->type == 's' || obj2->type == 's')
    return 1; // d and k are played
  if(obj1->type == 'r' || obj2->type == 'r')
    return 0; // suppose you play the easier...
  return ((tro_is_don(obj1) && tro_is_don(obj2)) ||
	  (tro_is_kat(obj1) && tro_is_kat(obj2)));
}

//---------------------------------------------------

int tro_are_same_density(struct tr_object * obj1,
			 struct tr_object * obj2)
{
  return (tro_are_same_type(obj1, obj2) &&
	  tro_are_same_hand(obj1, obj2));
}

//---------------------------------------------------
//---------------------------------------------------
//---------------------------------------------------

char * tro_str_state(struct tr_object * o)
{
  switch(o->ps)
    {
    case GREAT:
      return "Great";
    case GOOD:
      return "Good";
    case MISS:
      return "Miss";
    case BONUS:
      return "Bonus";
    default:
      return "_";
    }
}

//---------------------------------------------------

void tro_print_yaml(struct tr_object * o)
{
  fprintf(OUTPUT, "{");
  fprintf(OUTPUT, "offset: %d, ", o->offset);
  fprintf(OUTPUT, "density_star: %g, ", o->density_star);
  fprintf(OUTPUT, "pattern_star: %g, ", o->pattern_star);
  fprintf(OUTPUT, "reading_star: %g, ", o->reading_star);
  fprintf(OUTPUT, "accuracy_star: %g, ", o->accuracy_star);
  fprintf(OUTPUT, "final_star: %g", o->final_star);
  fprintf(OUTPUT, "}");
}

//---------------------------------------------------

void tro_print(struct tr_object * obj, int filter)
{
  if((filter & FILTER_BASIC) != 0)
    fprintf(OUTPUT_INFO, "%d\t%d\t%c\t%.3g\t%s\t",
	    obj->offset,
	    obj->rest,
	    obj->type,
	    obj->bpm_app,
	    tro_str_state(obj));
  if((filter & FILTER_BASIC_PLUS) != 0)
    fprintf(OUTPUT_INFO, "%d\t%d\t%d\t%c\t%.3g\t%s\t",
	    obj->offset,
	    obj->end_offset,
	    obj->rest,
	    obj->type,
	    obj->bpm_app,
	    tro_str_state(obj));
  if((filter & FILTER_ADDITIONNAL) != 0)
    fprintf(OUTPUT_INFO, "%d\t%d\t%g\t%g\t",
	    obj->l_hand,
	    obj->r_hand,
	    obj->obj_app,
	    obj->obj_dis);
  if((filter & FILTER_DENSITY) != 0)
    fprintf(OUTPUT_INFO, "%g\t%g\t%g\t",
	    obj->density_raw,
	    obj->density_color,
	    obj->density_star);
  if((filter & FILTER_READING) != 0)
    fprintf(OUTPUT_INFO, "%d\t%d\t%d\t%d\t%.0f.\t%.0f.\t%.0f.\t%.0f.\t%.3g\t",
	    obj->offset_app,
	    obj->offset_dis,
	    obj->visible_time,
	    obj->invisible_time,
	    obj->hidden,
	    obj->hide,
	    obj->fast,
	    obj->speed_change,
	    obj->reading_star);
  if((filter & FILTER_READING_PLUS) != 0)
    fprintf(OUTPUT_INFO, "%d\t%d\t%d\t%d\t%d\t%d\t%.2g\t%.2g\t%.2g\t%.2g\t%.3g\t",
	    obj->offset_app,
	    obj->end_offset_app,
	    obj->offset_dis,
	    obj->end_offset_dis,
	    obj->visible_time,
	    obj->invisible_time,
	    obj->hidden,
	    obj->hide,
	    obj->fast,
	    obj->speed_change,
	    obj->reading_star);
  if((filter & FILTER_ACCURACY) != 0)
    fprintf(OUTPUT_INFO, "%.3g\t%.3g\t%.3g\t",
	    obj->slow,
	    obj->hit_window,
	    obj->accuracy_star);
  if((filter & FILTER_PATTERN) != 0)
    {
      fprintf(OUTPUT_INFO, "%.3g\t%g\t",
	      obj->proba,
	      obj->pattern_star);
      int i = 0;
      while(obj->alt[i] >= 0)
	fprintf(OUTPUT_INFO, "%.3g\t", obj->alt[i++]);
    }
  if((filter & FILTER_STAR) != 0)
    fprintf(OUTPUT_INFO, "%.3g\t%.3g\t%.3g\t%.3g\t%.3g\t",
	    obj->density_star,
  	    obj->reading_star,
	    obj->pattern_star,
	    obj->accuracy_star,
	    obj->final_star);
  
  fprintf(OUTPUT_INFO, "\n");
}

//-------------------------------------------------
