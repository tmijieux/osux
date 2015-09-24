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

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "sum.h"
#include "stats.h"

#include "reading.h"

// superposition coeff
#define SUPERPOS_BIG   pow(2, 1 / 2.)
#define SUPERPOS_SMALL 1.

// hidding
#define HIDE_MAX 10000.
#define HIDE_EXP 500.

// speed 
#define BPM_CENTER   80.
#define BPM_MAX      1000.

#define SPEED_CENTER 10.
#define SPEED_MAX    1000.

// speed change
#define SPEED_CH_MAX     pow(10, 6)
#define SPEED_CH_TIME_0  1000.
#define SPEED_CH_VALUE_0 pow(10, -9)

// coeff for star
#define READING_STAR_COEFF_SUPERPOSED 0   // 50    lot 0
#define READING_STAR_COEFF_HIDE       0   // 5000  lot 0
#define READING_STAR_COEFF_HIDDEN     0   // 5000  lot 0
#define READING_STAR_COEFF_SPEED_CH   0   // 5000  lot 0
#define READING_STAR_COEFF_SPEED      1.  // 10000 no 0
#define READING_STAR_SCALING          100.

//-----------------------------------------------------

double tro_coeff_superpos (struct tr_object * obj)
{
  if (tro_is_big(obj))   
    return SUPERPOS_BIG;   // 'D' 'K' 'R'
  else
    return SUPERPOS_SMALL; // 'd' 'k' 'r' 's'
}

//-----------------------------------------------------

double tro_hidding (struct tr_object * obj1, struct tr_object * obj2)
{
  // int hide = obj1->end_offset_app - obj2->offset_app;
  // ^ not used
  int rest = obj2->offset - obj1->end_offset;
  double speed_change = obj1->bpm_app / obj2->bpm_app;
  double coeff = (tro_coeff_superpos(obj1) *
		  tro_coeff_superpos(obj2));
  if (speed_change > 1) 
    speed_change = obj2->bpm_app / obj1->bpm_app;
  // ^ True when obj1 hide obj2, so for hidden computation
  return HIDE_MAX * coeff * exp(-rest / HIDE_EXP) * speed_change;
}

//-----------------------------------------------------

double tro_speed (struct tr_object * obj)
{
  double bpm_app = obj->bpm_app;
  // decrease BPM to max
  if (bpm_app > BPM_MAX ||
      obj->visible_time == 0)
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
  
  // return SPEED_CENTER * exp(log(SPEED_MAX / SPEED_CENTER) *
  //  ((bpm_app - BPM_CENTER) /
  //  (BPM_MAX - BPM_CENTER)));
  
  return SPEED_CENTER * (1 + pow(bpm_app - BPM_CENTER,
				 (log(SPEED_MAX / SPEED_CENTER - 1) /
				  log(BPM_MAX - BPM_CENTER))));
}

//-----------------------------------------------------

double tro_speed_change (struct tr_object * obj1, struct tr_object * obj2)
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

void trm_compute_reading_hiding (struct tr_map * map)
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
	    sum_add(s_hidden, tro_hidding(&map->object[j],
					  &map->object[i]));
	}
      for (int j = i+1; j < map->nb_object; j++)
	{
	  int hide = (map->object[i].end_offset_app -
		      map->object[j].offset_app);
	  if (hide > 0)
	    sum_add(s_hide, tro_hidding(&map->object[i],
					&map->object[j]));
	}
      map->object[i].hide   = sum_compute(s_hide);
      map->object[i].hidden = sum_compute(s_hidden);
    }
}

//-----------------------------------------------------

void trm_compute_reading_superposed (struct tr_map * map)
{
  map->object[0].superposed = 0;
  for (int i = 1; i < map->nb_object; i++)
    {
      struct tr_object * obj = &map->object[i];
      double space_unit = (tro_coeff_superpos(&map->object[i-1]) *
			   tro_coeff_superpos(obj) *
			   mpb_to_bpm(obj->bpm_app) / 4.);
      if (obj->rest < space_unit)
	if (equal(obj->rest, space_unit))
	  obj->superposed = 0;
	else
	  obj->superposed = (tro_coeff_superpos(obj) *
			     100 * (space_unit - obj->rest) /
			     space_unit);
      else
	map->object[i].superposed = 0;
    }
}

//-----------------------------------------------------

void trm_compute_reading_speed (struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      struct tr_object * obj = &map->object[i];
      obj->speed = tro_speed(obj);
    }
}

//-----------------------------------------------------

void trm_compute_reading_speed_change (struct tr_map * map)
{
  map->object[0].speed_change = 0;
  for (int i = 1; i < map->nb_object; i++)
    {
      struct tr_object * obj = &map->object[i];
      obj->speed_change = 0;
      for (int j = 0; j < i; j++)
	{
	    obj->speed_change +=
	      tro_speed_change(&map->object[j],
			   &map->object[i]);
	}
    }
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_reading_star (struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].reading_star =
	(READING_STAR_COEFF_SUPERPOSED * map->object[i].superposed +
	 READING_STAR_COEFF_HIDE       * map->object[i].hide +
	 READING_STAR_COEFF_HIDDEN     * map->object[i].hidden +
	 READING_STAR_COEFF_SPEED_CH   * map->object[i].speed_change+
	 READING_STAR_COEFF_SPEED      * map->object[i].speed);
    }
  
  struct stats * stats = trm_stats_reading_star(map);
  double true_sum = (stats->mean /
		     (map->nb_object * READING_STAR_SCALING));

  free(stats);
  map->reading_star = true_sum;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_reading (struct tr_map * map)
{
  trm_compute_reading_hiding(map);
  trm_compute_reading_superposed(map);
  trm_compute_reading_speed(map);
  trm_compute_reading_speed_change(map);
  
  trm_compute_reading_star(map);
}
