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
#include "sum.h"

#include "reading.h"

// offset app & dis
#define NB_MAX_OBJ_BEFORE 12. // maybe 16
#define NB_MAX_OBJ_AFTER  0.  // maybe 1
#define OFFSET_MIN 10000
#define OFFSET_MAX (NB_MAX_OBJ_AFTER / NB_MAX_OBJ_BEFORE * OFFSET_MIN)

// superposition coeff
#define SUPERPOS_BIG   sqrt(2)
#define SUPERPOS_SMALL 1.

// hidding
#define HIDE_MAX 10000.
#define HIDE_EXP 500.

// speed 
#define BPM_CENTER   100.
#define BPM_MAX      1000.
#define SPEED_CENTER 10.
#define SPEED_MAX    1000.

// speed change
#define SPEED_CH_MAX     10000.
#define SPEED_CH_TIME_0  1000.
#define SPEED_CH_VALUE_0 pow(10, -9)

//-----------------------------------------------------

double tr_obj_coeff_superpos (struct tr_object * obj)
{
  if (tro_is_big(obj))   
    return SUPERPOS_BIG;   // 'D' 'K' 'R'
  else
    return SUPERPOS_SMALL; // 'd' 'k' 'r' 's'
}

//-----------------------------------------------------

double hidding (struct tr_object * obj1, struct tr_object * obj2)
{
  // int hide = obj1->end_offset_app - obj2->offset_app;
  // ^ not used
  int rest = obj2->offset - obj1->end_offset;
  double speed_change = obj1->bpm_app / obj2->bpm_app;
  double coeff = (tr_obj_coeff_superpos(obj1) *
		  tr_obj_coeff_superpos(obj2));
  if (speed_change > 1) 
    speed_change = obj2->bpm_app / obj1->bpm_app;
  // ^ True when obj1 hide obj2, so for hidden computation
  return HIDE_MAX * coeff * exp(-rest / HIDE_EXP) * speed_change;
}

//-----------------------------------------------------

double speed (double bpm_app)
{
  // decrease BPM to max
  if (bpm_app > BPM_MAX)
    bpm_app = BPM_MAX;
  
  // convert (0 to BPM_CENTER) to (BPM_CENTER to BPM_MAX) 
  if (bpm_app < BPM_CENTER)
    {
      //bpm_app = (BPM_CENTER - bpm_app) * (BPM_MAX / BPM_CENTER);
      bpm_app = (BPM_CENTER +
		 exp(log(BPM_CENTER - bpm_app) *
		     log(BPM_MAX - BPM_CENTER) /
		     log(BPM_CENTER)));
    }
  /*
    return SPEED_CENTER * exp(log(SPEED_MAX / SPEED_CENTER) *
			    ((bpm_app - BPM_CENTER) /
			    (BPM_MAX - BPM_CENTER)));
  */
  
  return SPEED_CENTER * (1 + pow(bpm_app - BPM_CENTER,
				 (log(SPEED_MAX / SPEED_CENTER - 1) /
				  log(BPM_MAX - BPM_CENTER))));
}

//-----------------------------------------------------

double speed_change (struct tr_object * obj1, struct tr_object * obj2)
{
  int rest = obj2->offset - obj1->end_offset;
  double coeff = obj1->bpm_app / obj2->bpm_app;
  if (coeff < 1) 
    coeff = obj2->bpm_app / obj1->bpm_app;

  return (SPEED_CH_MAX *
	  sqrt(coeff - 1) *
	  exp(log(SPEED_CH_VALUE_0) *
	      rest /
	      SPEED_CH_TIME_0));
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void compute_reading_offset (struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      struct tr_object * obj = &map->object[i];
      double space_unit = mpb_to_bpm(obj->bpm_app) / 4.;
      obj->offset_app = (obj->offset -
			 NB_MAX_OBJ_BEFORE * space_unit);
      // ^ wrong for spinner...
      obj->offset_dis = (obj->end_offset +
			 NB_MAX_OBJ_AFTER  * space_unit);
      // ^ will be useful for hidden :) but I hate hidden D:<

      // really far numbers... in case of really slow circle
      if (obj->offset_app < map->object[0].offset - OFFSET_MIN)
	{
	  obj->offset_app = map->object[0].offset - OFFSET_MIN;
	  obj->offset_dis = obj->end_offset       + OFFSET_MAX;
	}
      
      if (tro_is_slider(obj))
	{
	  int diff = obj->end_offset - obj->offset;
	  obj->end_offset_app = obj->offset_app + diff;
	  obj->end_offset_dis = obj->offset_dis + diff;
	}
      else if (obj->type == 's')
	{
	  int diff = obj->end_offset - obj->offset;
	  obj->end_offset_app = obj->offset_app;
	  obj->end_offset_dis = obj->offset_dis + diff;
	}
      else // d D k K
	{
	  obj->end_offset_app = obj->offset_app;
	  obj->end_offset_dis = obj->offset_dis;
	}
    }
}

//-----------------------------------------------------

void compute_reading_hiding (struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      void * s_hide   = sum_new(map->nb_object - i, DEFAULT);
      void * s_hidden = sum_new(i,                  DEFAULT);
      
      for (int j = 0; j < i; j++)
	{
	  int hidden = (map->object[j].end_offset_app -
			map->object[i].offset_app);
	  if (hidden > 0)
	    sum_add(s_hidden, hidding(&map->object[j],
				      &map->object[i]));
	}
      for (int j = i+1; j < map->nb_object; j++)
	{
	  int hide = (map->object[i].end_offset_app -
		      map->object[j].offset_app);
	  if (hide > 0)
	    sum_add(s_hide, hidding(&map->object[i],
				    &map->object[j]));
	}
      map->object[i].hide   = sum_compute(s_hide);
      map->object[i].hidden = sum_compute(s_hidden);
    }
}

//-----------------------------------------------------

void compute_reading_superposed (struct tr_map * map)
{
  map->object[0].superposed = 0;
  for (int i = 1; i < map->nb_object; i++)
    {
      struct tr_object * obj = &map->object[i];
      double space_unit = (tr_obj_coeff_superpos(&map->object[i-1]) *
			   tr_obj_coeff_superpos(obj) *
			   mpb_to_bpm(obj->bpm_app) / 4.);	
      if (obj->rest < space_unit)
	if (equal(obj->rest, space_unit))
	  obj->superposed = 0;
	else
	  obj->superposed = (tr_obj_coeff_superpos(obj) *
			     100 * (space_unit - obj->rest) /
			     space_unit);
      else
	map->object[i].superposed = 0;
    }
}

//-----------------------------------------------------

void compute_reading_speed (struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      struct tr_object * obj = &map->object[i];
      obj->speed = speed(obj->bpm_app);
    }
}

//-----------------------------------------------------

void compute_reading_speed_change (struct tr_map * map)
{
  map->object[0].speed_change = 0;
  for (int i = 1; i < map->nb_object; i++)
    {
      struct tr_object * obj = &map->object[i];
      obj->speed_change = 0;
      for (int j = 0; j < i; j++)
	{
	  /*double coeff = (map->object[i].bpm_app /
			  map->object[i-1].bpm_app);
			  if (!equal(coeff, 1))*/
	    obj->speed_change +=
	      speed_change(&map->object[j],
			   &map->object[i]);
	}
    }
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void compute_reading_star (struct tr_map * map)
{
  void * sum = sum_new(map->nb_object, PERF);
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].reading_star =
	(0.1 * (0 * map->object[i].superposed +   // 50    lot 0
		0 * map->object[i].hide +         // 5000  lot 0
		0 * map->object[i].hidden +       // 5000  lot 0
		0 * map->object[i].speed_change + // 5000  lot 0
		1 * map->object[i].speed));       // 10000 no 0
      sum_add(sum, map->object[i].reading_star);
    }
  double true_sum = sum_compute(sum) / (map->nb_object);
  map->reading_star = true_sum;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void compute_reading (struct tr_map * map)
{
  compute_reading_offset(map);
  compute_reading_hiding(map);
  compute_reading_superposed(map);
  compute_reading_speed(map);
  compute_reading_speed_change(map);
  
  compute_reading_star(map);
}
