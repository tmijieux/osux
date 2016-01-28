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
			     int (*is_in)(double, double, void *), 
			     void * arg);
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
static struct vector * SEEN_VECT;

// speed change
static double SPEED_CH_MAX;
static double SPEED_CH_TIME_0;
static double SPEED_CH_VALUE_0;

// coeff for star
static double READING_STAR_COEFF_SEEN;
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

  SEEN_VECT  = cst_vect(ht_cst, "vect_seen");
  HIDE_VECT  = cst_vect(ht_cst, "vect_hide");
  FAST_VECT  = cst_vect(ht_cst, "vect_fast");  
  SCALE_VECT = cst_vect(ht_cst, "vect_scale");

  SPEED_CH_MAX     = cst_f(ht_cst, "speed_ch_max");
  SPEED_CH_TIME_0  = cst_f(ht_cst, "speed_ch_time_0");
  SPEED_CH_VALUE_0 = cst_f(ht_cst, "speed_ch_value_0");

  READING_STAR_COEFF_SEEN       = cst_f(ht_cst, "star_seen");
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
  vect_free(SEEN_VECT);
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

static int pt_is_in_intersection(double x, double y,
				 struct tro_table * t)
{
  for(int i = 0; i < t->l; i++)
    {
      if(t->t[i] == NULL)
	continue;
      if(!pt_is_in_tro(x, y, t->t[i]))
	return 0;
    }
  return 1;
}

//-----------------------------------------------------

typedef int (*mc_cond)(double, double, void *);
static double tr_monte_carlo(int nb_pts,
			     double x1, double y1,
			     double x2, double y2,
			     int (*is_in)(double, double, void *), 
			     void * arg)
{
  int ok = 0;
  for(int i = 0; i < nb_pts; i++)
    {
      double x = x1 + RAND_DOUBLE * (x2 - x1); // x1 <= x <= x2
      double y = y1 + RAND_DOUBLE * (y2 - y1); // y1 <= y <= y2
      if(is_in(x, y, arg))
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
    { // parallelogram
      hide = ((fabs(x1 - x2) * fabs(y1 - y2)) -
	      ((fabs(x1 - x2) -
		(o1->end_offset_app - o2->offset_app)) *
	       fabs(y1 - y2)));
    }
  else
    { // complex figure
      struct tro_table * table = tro_table_from_vl(2, o1, o2);
      hide = tr_monte_carlo(MONTE_CARLO_NB_PT, x1, y1, x2, y2, 
			    (mc_cond)pt_is_in_intersection, 
			    table);
      tro_table_free(table);
    }

  if(tro_is_big(o1))
    hide *= TRO_BIG_SIZE;
  else
    hide *= TRO_SMALL_SIZE;
  return vect_poly2(HIDE_VECT, hide);  
}

//-----------------------------------------------------

static double tro_seen(struct tr_object * o, struct tr_object ** t,
		       int l)
{
  // all t is hiding o
  struct tr_object * copy = malloc(sizeof(*copy));
  *copy = *o;

  // same bpm_app because they are easy to compute
  int done = 0;
  for(int i = 0; i < l; i++)
    if(equal(o->bpm_app, t[i]->bpm_app))
      {
	if(t[i]->offset_dis > copy->offset_app)
	  {
	    copy->offset_app     = t[i]->offset_dis;
	    copy->end_offset_app = t[i]->end_offset_dis;
	  }
	t[i] = NULL;
	done++;
      }

  // seen 
  double seen = 
    ((copy->end_offset_dis - copy->offset_app) * 
     (copy->bpm_app * copy->end_offset_dis + copy->c_app)
     -
     (copy->end_offset_dis - copy->end_offset_app) * 
     (copy->bpm_app * copy->end_offset_dis + copy->c_app));
  if(tro_is_big(copy))
    seen *= TRO_BIG_SIZE;
  else
    seen *= TRO_SMALL_SIZE;

  // remove others superposition 
  if(done != l)
    {
      double x1 = copy->offset_app;
      double x2 = copy->end_offset_dis;
      double y1 = 0;
      double y2 = copy->bpm_app * copy->end_offset_dis + copy->c_app;
      struct tro_table * table = tro_table_from_array(t, l);
      seen -= tr_monte_carlo(MONTE_CARLO_NB_PT, x1, y1, x2, y2, 
			     (mc_cond)pt_is_in_intersection, 
			     table);
      tro_table_free(table);
    }
  else
    free(t);

  free(copy);
  //return seen;
  return vect_poly2(SEEN_VECT, seen);
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
      // list object that hide the i-th
      struct tr_object ** t1 = malloc(sizeof(*t1) * i);
      int l1 = 0;

      for (int j = 0; j < i; j++)
	{ // here if i has appeared before j
	  if(map->object[j].end_offset_app -
	     map->object[i].offset_app > 0) 
	    {
	      t1[l1] = &map->object[j];
	      l1++;
	    }
	}

      struct sum * s_hidden = sum_new(l1, DEFAULT);
      for(int k = 0; k < l1; k++)
	sum_add(s_hidden, tro_hide(t1[k], &map->object[i]));
      map->object[i].hidden = sum_compute(s_hidden);
      map->object[i].seen = tro_seen(&map->object[i], t1, l1);

      // list object that are hidden by the i-th
      struct tr_object ** t2 = malloc(sizeof(*t2)*map->nb_object-i);
      int l2 = 0;

      for (int j = i+1; j < map->nb_object; j++)
	{
	  if(map->object[i].end_offset_app -
	     map->object[j].offset_app > 0)
	    {
	      t2[l2] = &map->object[j];
	      l2++;
	    }
	}

      struct sum * s_hide = sum_new(l2, DEFAULT);
      for(int k = 0; k < l2; k++)
	sum_add(s_hide, tro_hide(&map->object[i], t2[k]));
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
	 (READING_STAR_COEFF_SEEN       * map->object[i].seen +
	  READING_STAR_COEFF_HIDE       * map->object[i].hide +
	  READING_STAR_COEFF_HIDDEN     * map->object[i].hidden +
	  READING_STAR_COEFF_SPEED_CH  * map->object[i].speed_change+
	  READING_STAR_COEFF_FAST       * map->object[i].fast));
    }
  map->reading_star = trm_weight_sum_reading_star(map, NULL);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void * trm_compute_reading(struct tr_map * map)
{
  if(!ht_cst)
    {
      tr_error("Unable to compute reading stars.");
      return NULL;
    }
  
  trm_compute_reading_hide(map);
  trm_compute_reading_fast(map);
  //trm_compute_reading_speed_change(map);
  trm_compute_reading_star(map);

  return NULL;
}
