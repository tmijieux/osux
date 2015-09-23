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
#include "stats.h"
#include "treatment.h"

#include "density.h"
#include "reading.h"
#include "accuracy.h"

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

void trm_compute_taiko_stars(struct tr_map * map, int mods)
{
  map->mods = mods;
  
  // compuatation
  trm_apply_mods(map);
  trm_treatment(map);
  trm_compute_density(map);
  trm_compute_reading(map);
  trm_compute_accuracy(map);

  trm_sort_density_star(map);
  
  // printing
  print_all_tr_object(map, FILTER_APPLY);
  print_map_star(map);
  print_map_final(map);
  
  // free
  free(map->title);
  free(map->creator);
  free(map->diff);  
  free(map->object);
  free(map);
}

//--------------------------------------------------

