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

#include "util/hash_table.h"
#include "util/list.h"
#include "util/yaml2.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "stats.h"
#include "cst_yaml.h"
#include "linear_fun.h"
#include "print.h"
#include "initializer.h"

#include "final_star.h"

static double weight_final_star(int i, double val);
static void tro_set_final_star(struct tr_object * obj);
static void trm_set_final_star(struct tr_map * map);

static void tro_apply_influence_coeff(struct tr_object *o, double c);
static double tro_influence_coeff(struct tr_object * o1,
				  struct tr_object * o2);
static void trm_set_influence(struct tr_map * map);

//-----------------------------------------------------

#define FINAL_FILE "final_cst.yaml"

static struct yaml_wrap * yw_fin;
static struct hash_table * ht_cst_fin;

static double DST_POW;
static double RDG_POW;
static double PTR_POW;
static double ACC_POW;

static struct linear_fun * INFLU_VECT;
static struct linear_fun * FINAL_SCALE_VECT;
static struct linear_fun * WEIGHT_VECT;

//-----------------------------------------------------

static void final_global_init(struct hash_table * ht_cst)
{
    INFLU_VECT = cst_lf(ht_cst, "vect_influence");
    WEIGHT_VECT = cst_lf(ht_cst, "vect_weight");
    FINAL_SCALE_VECT = cst_lf(ht_cst, "vect_scale");

    DST_POW = cst_f(ht_cst, "density_pow");
    RDG_POW = cst_f(ht_cst, "reading_pow");
    PTR_POW = cst_f(ht_cst, "pattern_pow");
    ACC_POW = cst_f(ht_cst, "accuracy_pow");
}


static void ht_cst_exit_final(void)
{
    yaml2_free(yw_fin);
    lf_free(FINAL_SCALE_VECT);
    lf_free(WEIGHT_VECT);
    lf_free(INFLU_VECT);
}

INITIALIZER(ht_cst_init_final)
{
    yw_fin = cst_get_yw(FINAL_FILE);
    ht_cst_fin = yw_extract_ht(yw_fin);
    if (ht_cst_fin != NULL)
	final_global_init(ht_cst_fin);
    atexit(ht_cst_exit_final);
}

//-----------------------------------------------------

static double weight_final_star(int i, double val)
{
    return lf_eval(WEIGHT_VECT, i) * val;
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
	lf_eval(FINAL_SCALE_VECT,
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

static void tro_apply_influence_coeff(struct tr_object * o, double c)
{
    o->density_star *= c;
    o->reading_star *= c;
    o->pattern_star *= c;
    o->accuracy_star *= c;
}

void tro_set_influence(struct tr_object * objs, int i, int nb)
{
    if(objs[i].ps == GREAT || objs[i].ps == BONUS) {
	return;
    }
    for(int j = i; j >= 0; j--) {
	double coeff = tro_influence_coeff(&objs[i], &objs[j]);
	if (coeff == 1)
	    break; /* influence will remain to 1 */
	tro_apply_influence_coeff(&objs[j], coeff);
    }
    for(int j = i+1; j < nb; j++) {
	double coeff = tro_influence_coeff(&objs[i], &objs[j]);
	if (coeff == 1)
	    break; /* influence will remain to 1 */
	tro_apply_influence_coeff(&objs[j], coeff);
    }
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_set_final_star(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	tro_set_final_star(&map->object[i]);
}

//-----------------------------------------------------

static void trm_set_influence(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	tro_set_influence(map->object, i, map->nb_object);
}

//-----------------------------------------------------

void trm_compute_final_star(struct tr_map * map)
{
    if(ht_cst_fin == NULL) {
	tr_error("Unable to compute final stars.");
	return;
    }
    
    trm_set_influence(map);
    trm_set_final_star(map);

    {
	map->density_star =
	    trm_weight_sum_density_star(map, weight_final_star);
	map->reading_star =
	    trm_weight_sum_reading_star(map, weight_final_star);
	map->pattern_star =
	    trm_weight_sum_pattern_star(map, weight_final_star);
	map->accuracy_star =
	    trm_weight_sum_accuracy_star(map, weight_final_star);
	map->final_star =
	    trm_weight_sum_final_star(map, weight_final_star);
    }
}
