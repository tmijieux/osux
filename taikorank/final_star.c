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
#include "linear_fun.h"
#include "print.h"

#include "final_star.h"

static void tro_set_final_star(struct tr_object * obj);
static void trm_set_final_star(struct tr_map * map);


static double tro_influence_coeff(struct tr_object * o1,
				  struct tr_object * o2);
static void trm_set_influence(struct tr_map * map);

//-----------------------------------------------------

#define FINAL_FILE "final_cst.yaml"

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;

static double DST_POW;
static double RDG_POW;
static double PTR_POW;
static double ACC_POW;

static struct linear_fun * INFLU_VECT;
static struct linear_fun * SCALE_VECT;

//-----------------------------------------------------

static void global_init(void)
{
    INFLU_VECT = cst_lf(ht_cst, "vect_influence");
    SCALE_VECT = cst_lf(ht_cst, "vect_scale");

    DST_POW = cst_f(ht_cst, "density_pow");
    RDG_POW = cst_f(ht_cst, "reading_pow");
    PTR_POW = cst_f(ht_cst, "pattern_pow");
    ACC_POW = cst_f(ht_cst, "accuracy_pow");
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
    lf_free(SCALE_VECT);
    lf_free(INFLU_VECT);
}

//-----------------------------------------------------

static void tro_set_final_star(struct tr_object * obj)
{
    if(obj->ps != GREAT) {
	obj->density_star = 0;
	obj->reading_star = 0;
	obj->pattern_star = 0;
	obj->accuracy_star = 0;
	obj->final_star = 0;
	return;
    }
    obj->final_star =
	lf_eval(SCALE_VECT,
		pow(obj->density_star,  DST_POW) *
		pow(obj->reading_star,  RDG_POW) *
		pow(obj->pattern_star,  PTR_POW) *
		pow(obj->accuracy_star, ACC_POW));
}

//-----------------------------------------------------

static double tro_influence_coeff(struct tr_object * o1,
				  struct tr_object * o2)
{
    return 1. - lf_eval(INFLU_VECT, fabs(o1->offset - o2->offset));
}

//-----------------------------------------------------

void tro_set_influence(struct tr_object * objs, int i, int nb)
{
    if(objs[i].ps == GREAT || objs[i].ps == BONUS) {
	return;
    }
    for(int j = 0; j < nb; j++) {
	double coeff = tro_influence_coeff(&objs[i], &objs[j]);
	objs[j].density_star *= coeff;
	objs[j].reading_star *= coeff;
	objs[j].pattern_star *= coeff;
	objs[j].accuracy_star *= coeff;
    }
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_set_final_star(struct tr_map * map)
{
    #pragma omp parallel for
    for(int i = 0; i < map->nb_object; i++) {
	tro_set_final_star(&map->object[i]);
    }
}

//-----------------------------------------------------

static void trm_set_influence(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++) {
	tro_set_influence(map->object, i, map->nb_object);
    }
}

//-----------------------------------------------------

void trm_compute_final_star(struct tr_map * map)
{
    if(!ht_cst) {
	tr_error("Unable to compute final stars.");
	return;
    }
    
    trm_set_influence(map);
    trm_set_final_star(map);

    #pragma omp parallel 
    #pragma omp single
    {
	#pragma omp task
	map->density_star = trm_weight_sum_density_star(map, NULL);
	#pragma omp task
	map->reading_star = trm_weight_sum_reading_star(map, NULL);
        #pragma omp task
	map->pattern_star = trm_weight_sum_pattern_star(map, NULL);
        #pragma omp task
	map->accuracy_star = trm_weight_sum_accuracy_star(map, NULL);
        #pragma omp task
	map->final_star = trm_weight_sum_final_star(map, NULL);
    }
}
