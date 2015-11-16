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

#include "density.h"

static double tro_coeff_density (struct tr_object * obj);
static double tro_density (struct tr_object * obj1,
			   struct tr_object * obj2);

static void trm_compute_density_raw (struct tr_map * map);
static void trm_compute_density_color (struct tr_map * map);

static void trm_compute_density_star (struct tr_map * map);


// coeff for density
#define DENSITY_X1  0.
#define DENSITY_Y1  10000.
#define DENSITY_X2  3000.
#define DENSITY_Y2  pow(10, -8)

// coefficient for object type, 1 is the maximum
#define DENSITY_NORMAL  1.
#define DENSITY_BIG     1.
#define DENSITY_BONUS   0.33

// coefficient for length weighting in density
#define DENSITY_LENGTH  0.2

// coeff for star
#define DENSITY_STAR_COEFF_COLOR 0.8
#define DENSITY_STAR_COEFF_RAW   0.2

// coeff for stats
#define DENSITY_COEFF_MEDIAN 0.7
#define DENSITY_COEFF_MEAN   0.
#define DENSITY_COEFF_D1     0.
#define DENSITY_COEFF_D9     0.
#define DENSITY_COEFF_Q1     0.3
#define DENSITY_COEFF_Q3     0.

// scaling
#define DENSITY_STAR_SCALING 15000.

// stats module
TRM_STATS_HEADER(density_star, DENSITY)

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_coeff_density (struct tr_object * obj)
{
  if (tro_is_bonus(obj)) 
    return DENSITY_BONUS;  // 'r' 'R' 's'
  if (tro_is_big(obj))   
    return DENSITY_BIG;    // 'D' 'K'
  else
    return DENSITY_NORMAL; // 'd' 'k'
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

  map->density_star = trm_stats_compute_density_star(map); 
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_density (struct tr_map * map)
{
  trm_compute_density_raw(map);
  trm_compute_density_color(map);
  trm_compute_density_star(map);
}