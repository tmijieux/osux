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
#include <string.h>
#include <stdarg.h>

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

#include "final_star.h"

//-----------------------------------------------------

#define FINAL_FILE  "final_cst.yaml"

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;

static struct vector * SCALE_VECT;

//-----------------------------------------------------

static void global_init(void)
{
    SCALE_VECT = cst_vect(ht_cst, "vect_scale");
}

__attribute__((constructor))
static void ht_cst_init_final(void)
{
    yw = cst_get_yw(FINAL_FILE);
    ht_cst = cst_get_ht(yw);
    if(ht_cst)
	global_init();
}

__attribute__((destructor))
static void ht_cst_exit_final(void)
{
    yaml2_free(yw);
    vect_free(SCALE_VECT);
}

//-----------------------------------------------------

void trm_compute_final_star(struct tr_map * map)
{
    if(!ht_cst) {
	tr_error("Unable to compute final stars.");
	return;
    }
  
#pragma omp parallel
#pragma omp for
    for(int i = 0; i < map->nb_object; i++) {
	if(map->object[i].ps != GREAT) {
	    map->object[i].density_star = 0;
	    map->object[i].reading_star = 0;
	    map->object[i].pattern_star = 0;
	    map->object[i].accuracy_star = 0;
	    map->object[i].final_star = 0;
	    continue;
	}
	map->object[i].final_star = 
	    vect_poly2(SCALE_VECT,
		       map->object[i].density_star *
		       map->object[i].reading_star *
		       map->object[i].pattern_star * 
		       map->object[i].accuracy_star);
    }
    map->final_star = trm_weight_sum_final_star(map, NULL);
}
