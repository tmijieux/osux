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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "util/hashtable/hash_table.h"
#include "util/list/list.h"
#include "yaml/yaml2.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "sum.h"
#include "stats.h"
#include "cst_yaml.h"
#include "print.h"

#include "reading.h"

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;

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

//--------------------------------------------------

#define READING_FILE  "reading_cst.yaml"
#define READING_STATS "reading_stats"

// superposition coeff
static double SUPERPOS_BIG;
static double SUPERPOS_SMALL;

// hiding
static double HIDE_MAX;
static double HIDE_EXP;

// slow
static double TIME_MIN;
static double TIME_CENTER;
static double TIME_MAX;
static double SLOW_MIN;
static double SLOW_MAX;

// fast
static double BPM_MIN;
static double BPM_CENTER;
static double BPM_MAX;
static double FAST_MIN;
static double FAST_MAX;

// speed change
static double SPEED_CH_MAX;
static double SPEED_CH_TIME_0;
static double SPEED_CH_VALUE_0;

// coeff for star
#define READING_STAR_COEFF_SUPERPOSED cst_f(ht_cst, "star_superpos") 
#define READING_STAR_COEFF_HIDE       cst_f(ht_cst, "star_hide")
#define READING_STAR_COEFF_HIDDEN     cst_f(ht_cst, "star_hidden")   
#define READING_STAR_COEFF_SPEED_CH   cst_f(ht_cst, "star_speed_ch")
#define READING_STAR_COEFF_SLOW       cst_f(ht_cst, "star_slow")
#define READING_STAR_COEFF_FAST       cst_f(ht_cst, "star_fast")

//-----------------------------------------------------

static void global_init(void)
{
  SUPERPOS_BIG   = cst_f(ht_cst, "superpos_big");
  SUPERPOS_SMALL = cst_f(ht_cst, "superpos_small");

  HIDE_MAX = cst_f(ht_cst, "hide_max");
  HIDE_EXP = cst_f(ht_cst, "hide_exp");

  TIME_MIN    = cst_f(ht_cst, "time_min");
  TIME_CENTER = cst_f(ht_cst, "time_center");
  TIME_MAX    = cst_f(ht_cst, "time_max");
  SLOW_MIN = cst_f(ht_cst, "slow_min");
  SLOW_MAX = cst_f(ht_cst, "slow_max");

  BPM_MIN    = cst_f(ht_cst, "bpm_min");
  BPM_CENTER = cst_f(ht_cst, "bpm_center");
  BPM_MAX    = cst_f(ht_cst, "bpm_max");
  FAST_MIN = cst_f(ht_cst, "fast_min");
  FAST_MAX = cst_f(ht_cst, "fast_max");

  SPEED_CH_MAX     = cst_f(ht_cst, "speed_ch_max");
  SPEED_CH_TIME_0  = cst_f(ht_cst, "speed_ch_time_0");
  SPEED_CH_VALUE_0 = cst_f(ht_cst, "speed_ch_value_0");
}

//-----------------------------------------------------

__attribute__((constructor))
static void ht_cst_init_reading(void)
{
  yw = cst_get_yw(READING_FILE);
  ht_cst = cst_get_ht(yw);
  if(ht_cst)
    global_init();
}

__attribute__((destructor))
static void ht_cst_exit_reading(void)
{
  //yaml2_free(yw);
}

//-----------------------------------------------------
//-----------------------------------------------------
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
  double bpm_app = obj->bpm_app;
  if (bpm_app > BPM_MAX ||
      obj->visible_time == 0) // 0 bpm ~ 2500 bpm
    bpm_app = BPM_MAX;

  return POLY_2_PT(bpm_app,
		   BPM_MIN, SLOW_MAX,
		   BPM_MAX, SLOW_MIN);
}

//-----------------------------------------------------

static double tro_fast(struct tr_object * obj)
{
  double time = obj->visible_time;
  if (time > TIME_MAX) 
    time = TIME_MAX; 
  else if (time < TIME_MIN) 
    time = TIME_MIN;

  return POLY_2_PT(time,
		   TIME_MIN, FAST_MAX,
		   TIME_MAX, FAST_MIN);
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
  struct stats * stats = trm_stats_reading_star(map);
  struct stats * coeff = cst_stats(ht_cst, READING_STATS);
  map->reading_star = stats_stars(stats, coeff);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_reading(struct tr_map * map)
{
  if(!ht_cst)
    {
      tr_error("Unable to compute reading stars.");
      return;
    }
  
  trm_compute_reading_hiding(map);
  trm_compute_reading_superposed(map);
  trm_compute_reading_slow(map);
  trm_compute_reading_fast(map);
  trm_compute_reading_speed_change(map);
  
  trm_compute_reading_star(map);
}
