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

#include "osux.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "stats.h"
#include "cst_yaml.h"
#include "linear_fun.h"
#include "print.h"
#include "final_star.h"

static double weight_final_star(int i, double val);

static void tro_apply_influence_coeff(struct tr_object *o, double c);
static double tro_influence_coeff(const struct tr_object * o1,
				  const struct tr_object * o2);

static void trm_set_influence(struct tr_map * map);
static void trm_set_final_star(struct tr_map * map);

static void trm_set_global_stars(struct tr_map * map);

//-----------------------------------------------------

#define FINAL_FILE "final_cst.yaml"

static struct yaml_wrap * yw_fin;
static osux_hashtable * ht_cst_fin;

static double DST_POW;
static double RDG_POW;
static double PTR_POW;
static double ACC_POW;

static struct linear_fun * FINAL_INFLU_LF;
static struct linear_fun * FINAL_SCALE_LF;
static struct linear_fun * WEIGHT_LF;

//-----------------------------------------------------

static void final_global_init(osux_hashtable * ht_cst)
{
    FINAL_INFLU_LF = cst_lf(ht_cst, "vect_influence");
    WEIGHT_LF = cst_lf(ht_cst, "vect_weight");
    FINAL_SCALE_LF = cst_lf(ht_cst, "vect_final_scale");

    DST_POW = cst_f(ht_cst, "density_pow");
    RDG_POW = cst_f(ht_cst, "reading_pow");
    PTR_POW = cst_f(ht_cst, "pattern_pow");
    ACC_POW = cst_f(ht_cst, "accuracy_pow");
}


static void ht_cst_exit_final(void)
{
    yaml2_free(yw_fin);
    lf_free(FINAL_SCALE_LF);
    lf_free(WEIGHT_LF);
    lf_free(FINAL_INFLU_LF);
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
//-----------------------------------------------------
//-----------------------------------------------------

static double weight_final_star(int i, double val)
{
    return lf_eval(WEIGHT_LF, i) * val;
}


//-----------------------------------------------------

static double tro_influence_coeff(const struct tr_object * o1,
				  const struct tr_object * o2)
{
    return lf_eval(FINAL_INFLU_LF, fabs(o1->offset - o2->offset));
}

//-----------------------------------------------------

static void tro_apply_influence_coeff(struct tr_object * o, double c)
{
    o->density_star  *= c;
    o->reading_star  *= c;
    o->pattern_star  *= c;
    o->accuracy_star *= c;
    o->final_star    *= c; // for score
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void tro_set_final_star(struct tr_object * o)
{
    if(o->ps != GREAT) {
	o->density_star  = 0;
	o->reading_star  = 0;
	o->pattern_star  = 0;
	o->accuracy_star = 0;
	o->final_star    = 0;
	return;
    }
    o->final_star =
	lf_eval(FINAL_SCALE_LF,
		pow(o->density_star,  DST_POW) *
		pow(o->reading_star,  RDG_POW) *
		pow(o->pattern_star,  PTR_POW) *
		pow(o->accuracy_star, ACC_POW));
}

//-----------------------------------------------------

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

static void trm_set_global_stars(struct tr_map * map)
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

//-----------------------------------------------------

void trm_compute_final_star(struct tr_map * map)
{
    if(ht_cst_fin == NULL) {
	tr_error("Unable to compute final stars.");
	return;
    }
    
    trm_set_influence(map);
    trm_set_final_star(map);
    trm_set_global_stars(map);
}
