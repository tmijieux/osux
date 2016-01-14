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

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "util/sum.h"
#include "util/hash_table.h"
#include "util/list.h"
#include "util/yaml2.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "stats.h"
#include "cst_yaml.h"
#include "vector.h"
#include "print.h"

#include "reading.h"

#define min(x, y) x < y ? x : y;
#define max(x, y) x > y ? x : y;
#define RAND_DOUBLE ((double) rand() / RAND_MAX)

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;

static double tro_hide(struct tr_object * o1, struct tr_object * o2);
static double tro_fast(struct tr_object * obj);
static int pt_is_in_tro(double x, double y, struct tr_object * o);

static double tr_monte_carlo(int nb_pts,
			     double x1, double y1,
			     double x2, double y2,
			     struct tr_object *o1,
			     struct tr_object *o2);
/*
static double tro_speed_change(struct tr_object * obj1,
				struct tr_object * obj2);
*/
static void trm_compute_reading_hide(struct tr_map * map);
static void trm_compute_reading_fast(struct tr_map * map);

static void trm_compute_reading_star(struct tr_map * map);

//--------------------------------------------------

#define READING_FILE  "reading_cst.yaml"

static int MONTE_CARLO_NB_PT;
static struct vector * HIDE_VECT;
static struct vector * FAST_VECT;

// speed change
static double SPEED_CH_MAX;
static double SPEED_CH_TIME_0;
static double SPEED_CH_VALUE_0;

// coeff for star
static double READING_STAR_COEFF_HIDE;
static double READING_STAR_COEFF_HIDDEN;
static double READING_STAR_COEFF_SPEED_CH;
static double READING_STAR_COEFF_FAST;
static struct vector * SCALE_VECT;

//-----------------------------------------------------

static void global_init(void)
{
  srand(time(NULL));
  MONTE_CARLO_NB_PT = cst_i(ht_cst, "monte_carlo_nb_pts");

  HIDE_VECT  = cst_vect(ht_cst, "vect_hide");
  FAST_VECT  = cst_vect(ht_cst, "vect_fast");  
  SCALE_VECT = cst_vect(ht_cst, "vect_scale");

  SPEED_CH_MAX     = cst_f(ht_cst, "speed_ch_max");
  SPEED_CH_TIME_0  = cst_f(ht_cst, "speed_ch_time_0");
  SPEED_CH_VALUE_0 = cst_f(ht_cst, "speed_ch_value_0");

  READING_STAR_COEFF_HIDE       = cst_f(ht_cst, "star_hide");
  READING_STAR_COEFF_HIDDEN     = cst_f(ht_cst, "star_hidden");   
  READING_STAR_COEFF_SPEED_CH   = cst_f(ht_cst, "star_speed_ch");
  READING_STAR_COEFF_FAST       = cst_f(ht_cst, "star_fast");
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
  yaml2_free(yw);
  vect_free(HIDE_VECT);
  vect_free(FAST_VECT);
  vect_free(SCALE_VECT);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static int pt_is_in_tro(double x, double y, struct tr_object * o)
{
  // y(x) = bpm_app * x + c_app
  // y(x) = bpm_app * x + c_end_app
  if(o->bpm_app * x + o->c_end_app <= y &&
     o->bpm_app * x + o->c_app     >= y)
    return 1;
  return 0;
}

//-----------------------------------------------------

static double tr_monte_carlo(int nb_pts,
			     double x1, double y1,
			     double x2, double y2,
			     struct tr_object *o1,
			     struct tr_object *o2)
{
  int ok = 0;
  for(int i = 0; i < nb_pts; i++)
    {
      double x = x1 + RAND_DOUBLE * (x2 - x1); // x1 <= x <= x2
      double y = y1 + RAND_DOUBLE * (y2 - y1); // y1 <= y <= y2
      if(pt_is_in_tro(x, y, o1) && pt_is_in_tro(x, y, o2))
	ok++;
    }
  return ((fabs(x1 - x2) * fabs(y1 - y2)) *
	  ((double) ok / (double) nb_pts));
}

//-----------------------------------------------------

static double tro_hide(struct tr_object * o1, struct tr_object * o2)
{
  // o1 is played before, so o1 is hiding o2
  double hide = 0;
  double x1 = max(o1->offset_app, o2->offset_app);
  double x2 = min(o1->end_offset_dis, o2->end_offset_dis);
  double y1 = 0;
  double y2 = o1->bpm_app * o1->end_offset_dis + o1->c_app;
  if(equal(o1->bpm_app, o2->bpm_app))
    // parallelogram
    hide = ((fabs(x1 - x2) * fabs(y1 - y2)) -
	    ((fabs(x1 - x2) -
	      (o1->end_offset_app - o2->offset_app)) *
	     fabs(y1 - y2)));
  else
    // complex figure
    hide = tr_monte_carlo(MONTE_CARLO_NB_PT, x1, y1, x2, y2, o1, o2);
  if(tro_is_big(o1))
    hide *= TRO_BIG_SIZE;
  else
    hide *= TRO_SMALL_SIZE;
  return vect_poly2(HIDE_VECT, hide);  
}

//-----------------------------------------------------

static double tro_fast(struct tr_object * obj)
{
  return vect_poly2(FAST_VECT, 
		    obj->visible_time + obj->invisible_time);
}

//-----------------------------------------------------
/*
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
*/
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_reading_hide(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      struct sum * s_hidden = sum_new(i, DEFAULT);
      for (int j = 0; j < i; j++)
	{
	  int hidden = (map->object[j].end_offset_app -
			map->object[i].offset_app);
	  if (hidden > 0) // here if i has appeared before j
	    sum_add(s_hidden, tro_hide(&map->object[j],
				       &map->object[i]));
	}
      map->object[i].hidden = sum_compute(s_hidden);

      struct sum * s_hide   = sum_new(map->nb_object - i, DEFAULT);
      for (int j = i+1; j < map->nb_object; j++)
	{
	  int hide = (map->object[i].end_offset_app -
		      map->object[j].offset_app);
	  if (hide > 0)
	    sum_add(s_hide, tro_hide(&map->object[i],
				     &map->object[j]));
	}
      map->object[i].hide = sum_compute(s_hide);
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
/*
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
*/
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_reading_star(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].reading_star = vect_poly2
	(SCALE_VECT,
	 (READING_STAR_COEFF_HIDE       * map->object[i].hide +
	  READING_STAR_COEFF_HIDDEN     * map->object[i].hidden +
	  READING_STAR_COEFF_SPEED_CH  * map->object[i].speed_change+
	  READING_STAR_COEFF_FAST       * map->object[i].fast));
    }
  map->reading_star = trm_weight_sum_reading_star(map, NULL);
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
  
  trm_compute_reading_hide(map);
  trm_compute_reading_fast(map);
  //trm_compute_reading_speed_change(map);
  trm_compute_reading_star(map);
}
