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

#include "final_star.h"

// coeff for stats
#define FINAL_COEFF_MEDIAN 0.7
#define FINAL_COEFF_MEAN   0.
#define FINAL_COEFF_D1     0.
#define FINAL_COEFF_D9     0.
#define FINAL_COEFF_Q1     0.3
#define FINAL_COEFF_Q3     0.

// scaling
#define FINAL_STAR_SCALING 2.

// stats module
TRM_STATS_HEADER(final_star, FINAL)

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_final_star (struct tr_map * map)
{
  for(int i = 0; i < map->nb_object; i++)
    {
      map->object[i].final_star =
	pow(map->object[i].density_star *
	    map->object[i].pattern_star,
	    0.25);
    }

  map->final_star = trm_stats_compute_final_star(map);
}

