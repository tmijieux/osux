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

#include "accuracy.h"

static void trm_compute_od_to_ms (struct tr_map * map);
static void trm_compute_spacing (struct tr_map * map);

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_od_to_ms (struct tr_map * map)
{
  map->great_ms = (int )(map->great_ms *
			 (MS_GREAT - (MS_COEFF_GREAT * map->od)));
  map->bad_ms =  (int) (map->bad_ms *
			(MS_BAD - (MS_COEFF_BAD * map->od)));
}

//-----------------------------------------------------

static void trm_compute_spacing (struct tr_map * map)
{
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_accuracy (struct tr_map * map)
{
  trm_compute_od_to_ms(map);
  trm_compute_spacing(map);
}
