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

#include "density.h"

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;

static double tro_get_coeff_density(struct tr_object * obj);
static double tro_density(struct tr_object * obj1,
			  struct tr_object * obj2);

static void tro_set_density_raw(struct tr_object * objs, int i);
static void trm_set_density_raw(struct tr_map * map);

static void tro_set_density_color(struct tr_object * objs, int i);
static void trm_set_density_color(struct tr_map * map);

static void tro_set_density_star(struct tr_object * obj);
static void trm_set_density_star(struct tr_map * map);

//--------------------------------------------------

#define DENSITY_FILE  "density_cst.yaml"

// coeff for density
static struct vector * DENSITY_VECT;

// coefficient for object type, 1 is the maximum
static double DENSITY_NORMAL;
static double DENSITY_BIG;
static double DENSITY_BONUS;

// coefficient for length weighting in density
static double DENSITY_LENGTH;

// coeff for star
static double DENSITY_STAR_COEFF_COLOR;
static double DENSITY_STAR_COEFF_RAW;
static struct vector * SCALE_VECT;

//-----------------------------------------------------

static void global_init(void)
{
    DENSITY_VECT = cst_vect(ht_cst, "vect_density");
    SCALE_VECT   = cst_vect(ht_cst, "vect_scale");

    DENSITY_NORMAL = cst_f(ht_cst, "density_normal");
    DENSITY_BIG    = cst_f(ht_cst, "density_big");
    DENSITY_BONUS  = cst_f(ht_cst, "density_bonus");

    DENSITY_LENGTH = cst_f(ht_cst, "density_length");
  
    DENSITY_STAR_COEFF_COLOR = cst_f(ht_cst, "star_color");
    DENSITY_STAR_COEFF_RAW   = cst_f(ht_cst, "star_raw");
}

//-----------------------------------------------------

__attribute__((constructor))
static void ht_cst_init_density(void)
{
    yw = cst_get_yw(DENSITY_FILE);
    ht_cst = cst_get_ht(yw);
    if(ht_cst)
	global_init();
}

__attribute__((destructor))
static void ht_cst_exit_density(void)
{
    yaml2_free(yw);
    vect_free(DENSITY_VECT);
    vect_free(SCALE_VECT);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_get_coeff_density(struct tr_object * obj)
{
    switch(obj->type) {
    case 'd':
    case 'k':
	return DENSITY_NORMAL;
    case 'D':
    case 'K':
	return DENSITY_BIG;
    case 'r':
    case 'R':
    case 's':
	return DENSITY_BONUS;
    default:
	tr_error("Wrong type %c.", obj->type);
	return -1;
    }
}

//-----------------------------------------------------

static double tro_density(struct tr_object * obj1, 
			  struct tr_object * obj2)
{
    double length = tro_get_length(obj1);
    double value  = vect_exp(DENSITY_VECT,
			     ((double) obj2->rest) + 
			     DENSITY_LENGTH * length);
    return tro_get_coeff_density(obj1) * value;
}

//-----------------------------------------------------

#define TRO_SET_DENSITY_TYPE(TYPE, TRO_TEST)			\
    static void tro_set_density_##TYPE(struct tr_object * objs,	\
				       int i)			\
    {								\
	if(objs[i].ps == MISS) {				\
	    objs[i].density_##TYPE = 0;				\
	    return;						\
	}							\
								\
	struct sum * sum = sum_new(i, DEFAULT);			\
	for(int j = 0; j < i; j++) {				\
	    if(objs[j].ps == MISS)				\
		continue;					\
	    if(TRO_TEST(&objs[i], &objs[j]))			\
		sum_add(sum, tro_density(&objs[j], &objs[i]));	\
	}							\
	double density = sum_compute(sum);			\
	density *= tro_get_coeff_density(&objs[i]);		\
	objs[i].density_##TYPE = density;			\
    }								\
    								\
    static void trm_set_density_##TYPE(struct tr_map * map)	\
    {								\
	map->object[0].density_##TYPE = 0;			\
	for(int i = 1; i < map->nb_object; i++)			\
	    tro_set_density_##TYPE (map->object, i);		\
    }

static int tro_true(struct tr_object * o1 __attribute__((unused)),
		    struct tr_object * o2 __attribute__((unused)))
{
    return 1;
}

TRO_SET_DENSITY_TYPE(raw,   tro_true)
TRO_SET_DENSITY_TYPE(color, tro_are_same_density)

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void tro_set_density_star(struct tr_object * obj)
{
    obj->density_star = vect_poly2
	(SCALE_VECT,
	 (DENSITY_STAR_COEFF_COLOR * obj->density_color +
	  DENSITY_STAR_COEFF_RAW   * obj->density_raw));

}

//-----------------------------------------------------

static void trm_set_density_star(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	tro_set_density_star(&map->object[i]);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_density(struct tr_map * map)
{
    if(!ht_cst) {
	tr_error("Unable to compute density stars.");
	return;
    }
  
    trm_set_density_raw(map);
    trm_set_density_color(map);
    trm_set_density_star(map);
}
