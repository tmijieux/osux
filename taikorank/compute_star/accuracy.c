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

#include <math.h>
#include <stdio.h>

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

#include "accuracy.h"
#include "spacing_count.h"

#define TIME_EQUAL_MS 12

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;

static double tro_slow(struct tr_object * obj);

static void trm_compute_slow(struct tr_map * map);
static void trm_compute_hit_window(struct tr_map * map);
static void trm_compute_spacing(struct tr_map * map);

//-----------------------------------------------------

#define ACCURACY_FILE  "accuracy_cst.yaml"

#define MS_GREAT       48
#define MS_COEFF_GREAT 3
#define MS_GOOD        108
#define MS_COEFF_GOOD  6
#define MS_MISS        500 // arbitrary
#define MS_COEFF_MISS  0   // arbitrary

static struct vector * SLOW_VECT;
static struct vector * HIT_WINDOW_VECT;
static struct vector * SPC_VECT;

static double ACCURACY_STAR_COEFF_SLOW;
static double ACCURACY_STAR_COEFF_HIT_WINDOW;
static double ACCURACY_STAR_COEFF_SPACING;
static struct vector * SCALE_VECT;

//-----------------------------------------------------

static void global_init(void)
{
  SLOW_VECT       = cst_vect(ht_cst, "vect_slow");
  HIT_WINDOW_VECT = cst_vect(ht_cst, "vect_hit_window");
  SPC_VECT        = cst_vect(ht_cst, "vect_spacing");
  SCALE_VECT      = cst_vect(ht_cst, "vect_scale");

  ACCURACY_STAR_COEFF_SLOW       = cst_f(ht_cst, "star_slow");
  ACCURACY_STAR_COEFF_HIT_WINDOW = cst_f(ht_cst, "star_hit_window");
  ACCURACY_STAR_COEFF_SPACING    = cst_f(ht_cst, "star_spacing");
}

//-----------------------------------------------------

__attribute__((constructor))
static void ht_cst_init_accuracy(void)
{
  yw = cst_get_yw(ACCURACY_FILE);
  ht_cst = cst_get_ht(yw);
  if(ht_cst)
    global_init();
}

__attribute__((destructor))
static void ht_cst_exit_accuracy(void)
{
  yaml2_free(yw);
  vect_free(SLOW_VECT);
  vect_free(SPC_VECT);
  vect_free(HIT_WINDOW_VECT);
  vect_free(SCALE_VECT);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_slow(struct tr_object * obj)
{
  return vect_poly2(SLOW_VECT, obj->bpm_app);
}

//-----------------------------------------------------

static double tro_spacing(struct tr_object * o1, 
			  struct tr_object * o2)
{
  int diff = o2->offset - o1->offset;
  return vect_poly2(SPC_VECT, diff);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_hit_window(struct tr_map * map)
{
  int great_ms = (int)(map->od_mod_mult *
		       (MS_GREAT - (MS_COEFF_GREAT * map->od)));
  int good_ms =  (int)(map->od_mod_mult *
		       (MS_GOOD  - (MS_COEFF_GOOD  * map->od)));
  int miss_ms =  (int)(map->od_mod_mult *
		       (MS_MISS  - (MS_COEFF_MISS  * map->od)));
  for(int i = 0; i < map->nb_object; i++)
    {
      if(map->object[i].ps == GREAT)
	map->object[i].hit_window = great_ms;
      else if(map->object[i].ps == GOOD)
	map->object[i].hit_window = good_ms;
      else // also bonus
	map->object[i].hit_window = miss_ms;
      map->object[i].hit_window = vect_exp
	(HIT_WINDOW_VECT, map->object[i].hit_window);
    }
}

//-----------------------------------------------------

static void trm_compute_slow(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      if(map->object[i].ps == MISS)
	{
	  map->object[i].slow = 0;
	  continue;
	}
      map->object[i].slow = tro_slow(&map->object[i]);
    }
}

//-----------------------------------------------------

static int equal_i(int x, int y)
{
  return abs(x - y) < TIME_EQUAL_MS;
}

static void trm_compute_spacing(struct tr_map * map)
{
  for(int i = 0; i < map->nb_object; i++)
    {
      struct tr_object * copy = tro_copy(map->object, i+1);
      tro_sort_rest(copy, i+1);

      struct list * l = spc_new();
      int last = 0;
      spc_add_f(l, copy[last].rest, tro_spacing(&copy[last], &map->object[i]));

      for(int j = 1; j < i+1; j++)
	{
	  if(equal_i(copy[last].rest, copy[j].rest))
	    spc_increase_f(l, copy[last].rest, tro_spacing(&copy[j], &map->object[i]));
	  else
	    {
	      spc_add_f(l, copy[j].rest, tro_spacing(&copy[j], &map->object[i]));
	      last = j;
	    }
	}
      // TODO: do something
      spc_print(l);

      spc_free(l);
      free(copy);
    }
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_accuracy_star(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].accuracy_star = vect_poly2
	(SCALE_VECT,
	 (ACCURACY_STAR_COEFF_SLOW * map->object[i].slow +
	  ACCURACY_STAR_COEFF_SPACING * map->object[i].spacing +
	  ACCURACY_STAR_COEFF_HIT_WINDOW*map->object[i].hit_window));
    }
  map->accuracy_star = trm_weight_sum_accuracy_star(map, NULL);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void * trm_compute_accuracy(struct tr_map * map)
{
  if(!ht_cst)
    {
      tr_error("Unable to compute accuracy stars.");
      return NULL;
    }

  trm_compute_hit_window(map);
  trm_compute_spacing(map);
  trm_compute_slow(map);
  trm_compute_accuracy_star(map);

  return NULL;
}
