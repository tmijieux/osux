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

#include "util/hashtable/hash_table.h"
#include "util/list/list.h"
#include "yaml/yaml2.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "sum.h"
#include "stats.h"
#include "cst_yaml.h"
#include "print.h"

#include "density.h"

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;

static double tro_coeff_density (struct tr_object * obj);
static double tro_density (struct tr_object * obj1,
			   struct tr_object * obj2);

static void trm_compute_density_raw (struct tr_map * map);
static void trm_compute_density_color (struct tr_map * map);

static void trm_compute_density_star (struct tr_map * map);

//--------------------------------------------------

#define DENSITY_FILE  "density_cst.yaml"
#define DENSITY_STATS "density_stats"

// coeff for density
static double DENSITY_X1;
static double DENSITY_Y1;
static double DENSITY_X2;
static double DENSITY_Y2;

// coefficient for object type, 1 is the maximum
static double DENSITY_NORMAL;
static double DENSITY_BIG;
static double DENSITY_BONUS;

// coefficient for length weighting in density
static double DENSITY_LENGTH;

// coeff for star
#define DENSITY_STAR_COEFF_COLOR cst_f(ht_cst, "star_color")
#define DENSITY_STAR_COEFF_RAW   cst_f(ht_cst, "star_raw")

//-----------------------------------------------------

static void global_init(void)
{
  DENSITY_X1 = cst_f(ht_cst, "density_x1");
  DENSITY_Y1 = cst_f(ht_cst, "density_y1");
  DENSITY_X2 = cst_f(ht_cst, "density_x2");
  DENSITY_Y2 = cst_f(ht_cst, "density_y2");

  DENSITY_NORMAL = cst_f(ht_cst, "density_normal");
  DENSITY_BIG    = cst_f(ht_cst, "density_big");
  DENSITY_BONUS  = cst_f(ht_cst, "density_bonus");

  DENSITY_LENGTH = cst_f(ht_cst, "density_length"); 
}

//-----------------------------------------------------

__attribute__((constructor))
static void ht_cst_init_density(void)
{
  yw = cst_get_yw(DENSITY_FILE);
  ht_cst = cst_get_ht(yw);
  if(ht_cst)
    global_init();
}

__attribute__((destructor))
static void ht_cst_exit_density(void)
{
  yaml2_free(yw);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_coeff_density (struct tr_object * obj)
{
  switch(obj->type)
    {
    case 'd':
    case 'k':
      return DENSITY_NORMAL;
    case 'D':
    case 'K':
      return DENSITY_BIG;
    case 'r':
    case 'R':
    case 's':
      return DENSITY_BONUS;
    default:
      tr_error("Wrong type %c.", obj->type);
      return -1;
    }
}

//-----------------------------------------------------

static double tro_density (struct tr_object * obj1, struct tr_object * obj2)
{
  int rest = obj2->offset - obj1->end_offset;
  int length = obj1->end_offset - obj1->offset;
  double coeff = tro_coeff_density(obj1);
  double value = EXP_2_PT(rest + DENSITY_LENGTH * length,
			  DENSITY_X1, DENSITY_Y1,
			  DENSITY_X2, DENSITY_X2);
  return coeff * value;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_density_raw (struct tr_map * map)
{
  map->object[0].density_raw = 0;
  for (int i = 1; i < map->nb_object; i++)
    {
      struct sum * sum = sum_new(i, DEFAULT);
      for (int j = 0; j < i; j++)
	{
	  sum_add(sum, tro_density(&map->object[j], &map->object[i]));
	}
      double density_raw = sum_compute(sum);
      density_raw *= tro_coeff_density(&map->object[i]);
      map->object[i].density_raw = density_raw;
    }
}

//-----------------------------------------------------

static void trm_compute_density_color (struct tr_map * map)
{
  map->object[0].density_color = 0;
  for (int i = 1; i < map->nb_object; i++)
    {
      struct sum * sum = sum_new(i, DEFAULT);
      for (int j = 0; j < i; j++)
	{
	  if (tro_are_same_density(&map->object[i], &map->object[j]))
	    sum_add(sum, tro_density(&map->object[j], &map->object[i]));
	}
      double density_color = sum_compute(sum);
      density_color *= tro_coeff_density(&map->object[i]);
      map->object[i].density_color = density_color;
    }
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_density_star (struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].density_star =
	(DENSITY_STAR_COEFF_COLOR * map->object[i].density_color +
	 DENSITY_STAR_COEFF_RAW   * map->object[i].density_raw);
    }
  struct stats * stats = trm_stats_density_star(map);
  struct stats * coeff = cst_stats(ht_cst, DENSITY_STATS);
  map->density_star = stats_stars(stats, coeff);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_density (struct tr_map * map)
{
  if(!ht_cst)
    {
      tr_error("Unable to compute density stars.");
      return;
    }
  
  trm_compute_density_raw(map);
  trm_compute_density_color(map);
  trm_compute_density_star(map);
}
