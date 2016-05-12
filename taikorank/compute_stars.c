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

static void trm_compute_separate(struct tr_map * map)
{
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
    trm_treatment(map);

    trm_compute_separate(map);
}

//--------------------------------------------------
