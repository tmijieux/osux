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

#include "osux.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "stats.h"
#include "cst_yaml.h"
#include "linear_fun.h"
#include "print.h"
#include "density.h"

static struct yaml_wrap * yw_dst;
static osux_hashtable * ht_cst_dst;

static inline int tro_true(const struct tr_object UNUSED(*o1), 
			   const struct tr_object UNUSED(*o2));
static inline int tro_are_same_density(const struct tr_object *o1, 
				       const struct tr_object *o2);

static double tro_get_coeff_density(const struct tr_object * obj);
static double tro_density(const struct tr_object * obj1,
			  const struct tr_object * obj2);

static void trm_set_density_raw(struct tr_map * map);
static void trm_set_density_color(struct tr_map * map);
static void trm_set_density_star(struct tr_map * map);

//--------------------------------------------------

#define DENSITY_FILE  "density_cst.yaml"

// coeff for density
static struct linear_fun * DENSITY_LF;

// coefficient for object type, 1 is the maximum
static double DENSITY_NORMAL;
static double DENSITY_BIG;
static double DENSITY_BONUS;

// coefficient for length weighting in density
static double DENSITY_LENGTH;

// coeff for star
static double DENSITY_STAR_COEFF_COLOR;
static double DENSITY_STAR_COEFF_RAW;
static struct linear_fun * DENSITY_SCALE_LF;

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void density_global_init(osux_hashtable * ht_cst)
{
    DENSITY_LF       = cst_lf(ht_cst, "vect_density");
    DENSITY_SCALE_LF = cst_lf(ht_cst, "vect_scale");

    DENSITY_NORMAL = cst_f(ht_cst, "density_normal");
    DENSITY_BIG    = cst_f(ht_cst, "density_big");
    DENSITY_BONUS  = cst_f(ht_cst, "density_bonus");

    DENSITY_LENGTH = cst_f(ht_cst, "density_length");
  
    DENSITY_STAR_COEFF_COLOR = cst_f(ht_cst, "star_color");
    DENSITY_STAR_COEFF_RAW   = cst_f(ht_cst, "star_raw");
}

//-----------------------------------------------------

static void ht_cst_exit_density(void)
{
    yaml2_free(yw_dst);
    lf_free(DENSITY_LF);
    lf_free(DENSITY_SCALE_LF);
}

INITIALIZER(ht_cst_init_density)
{
    yw_dst = cst_get_yw(DENSITY_FILE);
    ht_cst_dst = yw_extract_ht(yw_dst);
    if (ht_cst_dst != NULL)
	density_global_init(ht_cst_dst);
    atexit(ht_cst_exit_density);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_get_coeff_density(const struct tr_object * o)
{
    if (tro_is_bonus(o))
	return DENSITY_BONUS;
    else if (tro_is_big(o))
	return DENSITY_BIG;
    else
	return DENSITY_NORMAL;
}

//-----------------------------------------------------

static double tro_density(const struct tr_object * obj1, 
			  const struct tr_object * obj2)
{
    double value  = lf_eval(DENSITY_LF, 
			    ((double) obj2->end_offset - obj1->offset) + 
			    DENSITY_LENGTH * obj1->length);
    return tro_get_coeff_density(obj1) * value;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

#define TRO_SET_DENSITY_TYPE(TYPE, TRO_TEST)			\
    void tro_set_density_##TYPE (struct tr_object * o, int i)	\
    {								\
	if(o->ps == MISS) {					\
	    o->density_##TYPE = 0;				\
	    return;						\
	}							\
								\
	double sum = 0;						\
	for(int j = i-1; j >= 0; j--) {				\
	    if(o->objs[j].ps == MISS)				\
		continue;					\
	    if(TRO_TEST(&o->objs[j], o)) {			\
		/* each object give a density value */		\
		/* based on the time between the two objects */	\
		double d = tro_density(&o->objs[j], o);		\
		if (d == 0)					\
		    break; /* j-- density won't increase */	\
		sum += d;					\
	    }							\
	}							\
	sum *= tro_get_coeff_density(o);			\
	o->density_##TYPE = sum;				\
    }								\
    								\
    static void trm_set_density_##TYPE(struct tr_map * map)	\
    {								\
	map->object[0].density_##TYPE = 0;			\
	for(int i = 1; i < map->nb_object; i++)			\
	    tro_set_density_##TYPE (&map->object[i], i);	\
    }

static inline int tro_true(const struct tr_object UNUSED(*o1), 
			   const struct tr_object UNUSED(*o2))
{
    // for hands density, all objects give a density value
    return 1;
}

static inline int tro_are_same_density(const struct tr_object *o1, 
				       const struct tr_object *o2)
{
    // for finger density, only objects played with the same finger
    // give a density value.
    return tro_are_same_type(o1, o2) && tro_are_same_hand(o1, o2);
}

TRO_SET_DENSITY_TYPE(raw,   tro_true)
TRO_SET_DENSITY_TYPE(color, tro_are_same_density)

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void tro_set_density_star(struct tr_object * obj)
{
    obj->density_star = lf_eval
	(DENSITY_SCALE_LF,
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
    if(ht_cst_dst == NULL) {
	tr_error("Unable to compute density stars.");
	return;
    }

    /*
      Computation is in two parts:
      - raw, can be interpreted as hand strain. Every object give 
        strain. 
      - color, can be interpreted as finger strain. Only object 
        played on the same key give strain.
     */
    trm_set_density_raw(map);
    trm_set_density_color(map);

    trm_set_density_star(map);
}
