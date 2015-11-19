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
#include "interpolation.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "sum.h"
#include "stats.h"

#include "reading.h"

static double tro_coeff_superpos(struct tr_object * obj);
static double tro_hiding(struct tr_object * obj1,
			   struct tr_object * obj2);
static double tro_speed_change(struct tr_object * obj1,
				struct tr_object * obj2);
static double tro_slow(struct tr_object * obj);
static double tro_fast(struct tr_object * obj);

static void trm_compute_reading_hiding(struct tr_map * map);
static void trm_compute_reading_superposed(struct tr_map * map);
static void trm_compute_reading_slow(struct tr_map * map);
static void trm_compute_reading_fast(struct tr_map * map);

static void trm_compute_reading_star(struct tr_map * map);

// superposition coeff
#define SUPERPOS_BIG   sqrt(2)
#define SUPERPOS_SMALL 1.

// hiding
#define HIDE_MAX 10000.
#define HIDE_EXP 500.

// slow
#define TIME_MIN    100.   // human reaction time ~2500bpm app
#define TIME_CENTER 3200.  // 3200ms ~ 80bpm app
#define TIME_MAX    60000. // max slow considered

#define SLOW_MIN 0.
#define SLOW_MAX 10000.

// fast
#define BPM_MIN      0.
#define BPM_CENTER   80.
#define BPM_MAX      2500.

#define FAST_MIN 1.
#define FAST_MAX 10000.

// speed change
#define SPEED_CH_MAX     pow(10, 6)
#define SPEED_CH_TIME_0  1000.
#define SPEED_CH_VALUE_0 pow(10, -9)

// coeff for star
#define READING_STAR_COEFF_SUPERPOSED 0   
#define READING_STAR_COEFF_HIDE       0   
#define READING_STAR_COEFF_HIDDEN     0   
#define READING_STAR_COEFF_SPEED_CH   0   
#define READING_STAR_COEFF_SLOW       1.
#define READING_STAR_COEFF_FAST       1.

// coeff for stats
#define READING_COEFF_MEDIAN 0.7
#define READING_COEFF_MEAN   0.
#define READING_COEFF_D1     0.
#define READING_COEFF_D9     0.
#define READING_COEFF_Q1     0.3
#define READING_COEFF_Q3     0.

// scaling
#define READING_STAR_SCALING 100.

// stats module
TRM_STATS_HEADER(reading_star, READING)

//-----------------------------------------------------

static double tro_coeff_superpos(struct tr_object * obj)
{
  if (tro_is_big(obj))   
    return SUPERPOS_BIG;   // 'D' 'K' 'R'
  else
    return SUPERPOS_SMALL; // 'd' 'k' 'r' 's'
}

//-----------------------------------------------------

static double tro_hiding(struct tr_object * obj1, struct tr_object * obj2)
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

static double tro_slow(struct tr_object * obj)
{
  double time = obj->visible_time;
  if (time > TIME_MAX) 
    time = TIME_MAX; 
  else if (time < TIME_MIN) 
    time = TIME_MIN;

  return POLY_2_PT(time,
		   TIME_MIN, SLOW_MIN,
		   TIME_MAX, SLOW_MAX);
}

//-----------------------------------------------------

static double tro_fast(struct tr_object * obj)
{
  double bpm_app = obj->bpm_app;
  if (bpm_app > BPM_MAX ||
      obj->visible_time == 0)
    bpm_app = BPM_MAX;

  return POLY_2_PT(bpm_app,
		   BPM_MIN, FAST_MIN,
		   BPM_MAX, FAST_MAX);
}

//-----------------------------------------------------

static double tro_speed_change(struct tr_object * obj1,
			       struct tr_object * obj2)
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

static void trm_compute_reading_hiding(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      struct sum * s_hide   = sum_new(map->nb_object - i, DEFAULT);
      struct sum * s_hidden = sum_new(i,                  DEFAULT);
      
      for (int j = 0; j < i; j++)
	{
	  int hidden = (map->object[j].end_offset_app -
			map->object[i].offset_app);
	  if (hidden > 0)
	    sum_add(s_hidden, tro_hiding(&map->object[j],
					  &map->object[i]));
	}
      for (int j = i+1; j < map->nb_object; j++)
	{
	  int hide = (map->object[i].end_offset_app -
		      map->object[j].offset_app);
	  if (hide > 0)
	    sum_add(s_hide, tro_hiding(&map->object[i],
					&map->object[j]));
	}
      map->object[i].hide   = sum_compute(s_hide);
      map->object[i].hidden = sum_compute(s_hidden);
    }
}

//-----------------------------------------------------

static void trm_compute_reading_superposed(struct tr_map * map)
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

static void trm_compute_reading_slow(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].slow = tro_slow(&map->object[i]);
    }
}

//-----------------------------------------------------

static void trm_compute_reading_fast(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].fast = tro_fast(&map->object[i]);
    }
}

//-----------------------------------------------------

static void trm_compute_reading_speed_change(struct tr_map * map)
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

static void trm_compute_reading_star(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].reading_star =
	(READING_STAR_COEFF_SUPERPOSED * map->object[i].superposed +
	 READING_STAR_COEFF_HIDE       * map->object[i].hide +
	 READING_STAR_COEFF_HIDDEN     * map->object[i].hidden +
	 READING_STAR_COEFF_SPEED_CH   * map->object[i].speed_change+
	 READING_STAR_COEFF_SLOW       * map->object[i].slow +
	 READING_STAR_COEFF_FAST       * map->object[i].fast);
    }

  map->reading_star = trm_stats_compute_reading_star(map); 
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_reading(struct tr_map * map)
{
  trm_compute_reading_hiding(map);
  trm_compute_reading_superposed(map);
  trm_compute_reading_slow(map);
  trm_compute_reading_fast(map);
  trm_compute_reading_speed_change(map);
  
  trm_compute_reading_star(map);
}
