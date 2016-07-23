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
    trm_set_combo(map);
    trm_set_rest(map);
    trm_set_hand(map);
    double * ggm_val = trm_get_ggm_val(map);
    int nb = map->nb_object;

    {
	for (int i = 0; i < nb; i++)
	    #pragma omp task
	    {
		struct tr_object * o = &map->object[i];
	    
		// treatment
		tro_set_length(o);
		tro_set_app_dis_offset(o);
		// pattern
		tro_set_pattern_proba(o, i);
		tro_set_type(o);
		// accuracy
		tro_set_slow(o);
		tro_set_hit_window(o, ggm_val);
	    }
        #pragma omp taskwait

	for (int i = 0; i < nb; i++)
	    #pragma omp task
	    {
		struct tr_object * o = &map->object[i];

		// treatment
		tro_set_line_coeff(o);
		// density
		tro_set_density_raw(o, i);
		tro_set_density_color(o, i);
		// pattern
		tro_set_patterns(o, i, nb);
		// accuracy
		tro_set_spacing(o, i);
	    }
        #pragma omp taskwait

	for (int i = 0; i < nb; i++)
	    #pragma omp task
	    {
		struct tr_object * o = &map->object[i];

		// density
		tro_set_density_star(o);
		// reading
		tro_set_seen(o, i);
		// pattern
		tro_set_pattern_freq(o, i);
		// accuracy
		tro_set_accuracy_star(o);
	    }
        #pragma omp taskwait

	for (int i = 0; i < nb; i++)
	    #pragma omp task
	    {
		struct tr_object * o = &map->object[i];

		// reading
		tro_set_reading_star(o);
		// pattern
		tro_set_pattern_star(o);
		tro_free_patterns(o);
	    }
        #pragma omp taskwait
    }

    trm_compute_final_star(map);
}
*/
//--------------------------------------------------

static void trm_compute_separated(struct tr_map * map)
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

    trm_compute_separated(map);
    //trm_compute_grouped(map);
}

//--------------------------------------------------
