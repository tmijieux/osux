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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"

#include "print.h"

#include "mods.h"
#include "treatment.h"

#include "density.h"
#include "reading.h"
#include "pattern.h"
#include "accuracy.h"
#include "final_star.h"

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

void trm_compute_taiko_stars(const struct tr_map * map, int mods)
{
  struct tr_map * map_copy = trm_copy(map);
  map_copy->mods = mods;  
  
  // computation
  trm_apply_mods(map_copy);
  trm_treatment(map_copy);
  
  //trm_compute_density(map_copy);
  trm_compute_reading(map_copy);
  //trm_compute_pattern(map_copy);
  //trm_compute_accuracy(map_copy);
  //trm_compute_final_star(map_copy);

  // printing
  print_all_tr_object(map_copy, FILTER_APPLY);
  //print_map_star(map_copy);
  print_map_final(map_copy);

  // free
  trm_free(map_copy);
}

//--------------------------------------------------

struct tr_map * trm_copy (const struct tr_map * map)
{
  struct tr_map * copy = malloc(sizeof(*copy));
  memcpy(copy, map, sizeof(*map));

  copy->object = malloc(sizeof(map->object[0]) * map->nb_object);
  memcpy(copy->object, map->object,
	  sizeof(map->object[0]) * map->nb_object);
  
  copy->title   = strdup(map->title);
  copy->creator = strdup(map->creator);
  copy->diff    = strdup(map->diff);
  return copy;
}

//--------------------------------------------------

void trm_free (struct tr_map * map)
{
  free(map->title);
  free(map->creator);
  free(map->diff);  
  free(map->object);
  free(map);
}
