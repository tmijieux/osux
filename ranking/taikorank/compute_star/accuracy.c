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

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;

static double tro_slow(struct tr_object * obj);

static void trm_compute_slow(struct tr_map * map);
static void trm_compute_od_to_ms(struct tr_map * map);
static void trm_compute_spacing(struct tr_map * map);

//-----------------------------------------------------

#define ACCURACY_FILE  "accuracy_cst.yaml"

static struct vector * SLOW_VECT;

static double ACCURACY_STAR_COEFF_SLOW;
static struct vector * SCALE_VECT;

//-----------------------------------------------------

static void global_init(void)
{
  SLOW_VECT = cst_vect(ht_cst, "vect_slow");
  SCALE_VECT = cst_vect(ht_cst, "vect_scale");

  ACCURACY_STAR_COEFF_SLOW = cst_f(ht_cst, "star_slow");
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
  if(yw)
    yaml2_free(yw);
  free(SLOW_VECT);
  free(SCALE_VECT);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_slow(struct tr_object * obj)
{
  return vect_poly2(SLOW_VECT, obj->bpm_app);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_od_to_ms(struct tr_map * map)
{
  map->great_ms = (int)(map->great_ms *
			(MS_GREAT - (MS_COEFF_GREAT * map->od)));
  map->bad_ms =  (int)(map->bad_ms *
		       (MS_BAD - (MS_COEFF_BAD * map->od)));
}

//-----------------------------------------------------

static void trm_compute_slow(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].slow = tro_slow(&map->object[i]);
    }
}

//-----------------------------------------------------

static void trm_compute_spacing(struct tr_map * map)
{
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
	 (ACCURACY_STAR_COEFF_SLOW * map->object[i].slow));
    }
  map->accuracy_star = trm_weight_sum_accuracy_star(map, NULL);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_accuracy(struct tr_map * map)
{
  if(!ht_cst)
    {
      tr_error("Unable to compute accuracy stars.");
      return;
    }

  trm_compute_od_to_ms(map);
  trm_compute_spacing(map);
  trm_compute_slow(map);
  trm_compute_accuracy_star(map);
}
