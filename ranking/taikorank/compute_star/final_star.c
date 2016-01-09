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

#include "util/sum/sum.h"
#include "util/hashtable/hash_table.h"
#include "util/list/list.h"
#include "yaml/yaml2.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "stats.h"
#include "cst_yaml.h"
#include "print.h"

#include "final_star.h"

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;

static double SCALE_X1;
static double SCALE_Y1;
static double SCALE_X2;
static double SCALE_Y2;

// stats
static struct stats * STATS_COEFF;

#define FINAL_FILE  "final_cst.yaml"
#define FINAL_STATS "final_stats"

//-----------------------------------------------------

static void global_init(void)
{
  SCALE_X1 = cst_f(ht_cst, "scale_x1");
  SCALE_Y1 = cst_f(ht_cst, "scale_y1");
  SCALE_X2 = cst_f(ht_cst, "scale_x2");
  SCALE_Y2 = cst_f(ht_cst, "scale_y2");

  STATS_COEFF = cst_stats(ht_cst, FINAL_STATS);
}

__attribute__((constructor))
static void ht_cst_init_final(void)
{
  yw = cst_get_yw(FINAL_FILE);
  ht_cst = cst_get_ht(yw);
  global_init();
}

__attribute__((destructor))
static void ht_cst_exit_final(void)
{
  yaml2_free(yw);
  free(STATS_COEFF);
}

//-----------------------------------------------------

void trm_compute_final_star(struct tr_map * map)
{
  if(!ht_cst)
    {
      tr_error("Unable to compute reading stars.");
      return;
    }
  
  for(int i = 0; i < map->nb_object; i++)
    {
      map->object[i].final_star =
	POLY_2_PT(map->object[i].density_star *
		  map->object[i].pattern_star,
		  SCALE_X1, SCALE_Y1,
		  SCALE_X2, SCALE_Y2);
    }
  /*
  struct stats * stats = trm_stats_final_star(map);
  map->final_star = stats_stars(stats, STATS_COEFF);
  */
  map->final_star = trm_weight_sum_final_star(map, NULL);
}
