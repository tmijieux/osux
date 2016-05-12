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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "taiko_ranking_object.h"
#include "taiko_ranking_map.h"
#include "treatment.h"
#include "tr_mods.h"
#include "compute_stars.h"

#include "density.h"
#include "reading.h"
#include "pattern.h"
#include "accuracy.h"
#include "final_star.h"

//--------------------------------------------------
/*
static void trm_compute_grouped(struct tr_map * map)
{
    trm_treatment(map);
    
    struct tr_object * objs = map->object;
    int * ggm_ms = trm_get_ggm_ms(map);
    
    for (int i = 0; i < map->nb_object; i++) {
	tro_set_pattern_proba(objs, i);
    }
    trm_set_patterns(map);
    
    #pragma omp parallel for
    for (int i = 0; i < map->nb_object; i++) {
	struct tr_object * o = &map->object[i];

	tro_set_density_raw(objs, i);
	tro_set_density_color(objs, i);

	tro_set_seen(objs, i);
	    
	tro_set_pattern_freq(objs, i);
	    
	tro_set_hit_window(o, ggm_ms);
	tro_set_slow(o);
	tro_set_spacing(objs, i);

	tro_set_density_star(o);
	tro_set_reading_star(o);
	tro_set_pattern_star(o);
	tro_set_accuracy_star(o);
	//tro_free_patterns(o);
    }
    trm_compute_final_star(map);
}
*/
//--------------------------------------------------

static void trm_compute_separate(struct tr_map * map)
{
    trm_treatment(map);
    {
        #pragma omp task
	trm_compute_density(map);
        #pragma omp task
	trm_compute_reading(map);
        #pragma omp task
	trm_compute_pattern(map);
        #pragma omp task
	trm_compute_accuracy(map);
    }
    #pragma omp taskwait

    trm_compute_final_star(map);
}

//--------------------------------------------------

void trm_compute_stars(struct tr_map * map)
{
    if((map->mods & MODS_FL) != 0)
	trm_apply_mods_FL(map);

    trm_compute_separate(map);
}

//--------------------------------------------------
