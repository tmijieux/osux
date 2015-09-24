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

#include "density.h"

// coeff for density
// LOCAL density for two object on the same time
#define DENSITY_MAX     10000.
// when the rest time is DENSITY_TIME_0, the exp value will be
// DENSITY_VALUE_0
#define DENSITY_TIME_0  4000.
#define DENSITY_VALUE_0 pow(10, -9)

// coefficient for object type, 1 is the maximum
#define DENSITY_NORMAL  1.
#define DENSITY_BIG     1.
#define DENSITY_BONUS   0.33

// coefficient for length weighting in density
#define DENSITY_LENGTH  0.3

// coeff for star
#define DENSITY_STAR_COEFF_COLOR 0.8
#define DENSITY_STAR_COEFF_RAW   0.2
#define DENSITY_STAR_SCALING     700.

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

double tro_coeff_density (struct tr_object * obj)
{
  if (tro_is_bonus(obj)) 
    return DENSITY_BONUS;  // 'r' 'R' 's'
  if (tro_is_big(obj))   
    return DENSITY_BIG;    // 'D' 'K'
  else
    return DENSITY_NORMAL; // 'd' 'k'
}

//-----------------------------------------------------

double tro_density (struct tr_object * obj1, struct tr_object * obj2)
{
  int rest = obj2->offset - obj1->end_offset;
  double coeff = tro_coeff_density(obj1);
  int length = obj1->end_offset - obj1->offset;
  return (DENSITY_MAX * coeff *
	  exp((rest + DENSITY_LENGTH * length) *
	      (log(DENSITY_VALUE_0) / DENSITY_TIME_0)));
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_density_raw (struct tr_map * map)
{
  map->object[0].density_raw = 0;
  for (int i = 1; i < map->nb_object; i++)
    {
      void * sum = sum_new(i, DEFAULT);
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

void trm_compute_density_color (struct tr_map * map)
{
  map->object[0].density_color = 0;
  for (int i = 1; i < map->nb_object; i++)
    {
      void * sum = sum_new(i, DEFAULT);
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

void trm_compute_density_star (struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].density_star =
	(DENSITY_STAR_COEFF_COLOR * map->object[i].density_color +
	 DENSITY_STAR_COEFF_RAW   * map->object[i].density_raw);
    }

  struct stats * stats = trm_stats_density_star(map);
  double true_sum = (stats->mean /
		     (map->nb_object * DENSITY_STAR_SCALING));
  
  free(stats);
  map->density_star = true_sum; 
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
