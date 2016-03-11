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

#include "accuracy.h"
#include "spacing_count.h"

#define TIME_EQUAL_MS 12

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;

static double tro_slow(struct tr_object * obj);

static void tro_set_slow(struct tr_object * obj);
static void trm_set_slow(struct tr_map * map);

static void tro_set_hit_window(struct tr_object * obj, int ggm_ms[]);
static void trm_set_hit_window(struct tr_map * map);

static void tro_set_spacing(struct tr_object * objs, int i);
static void trm_set_spacing(struct tr_map * map);

static void tro_set_accuracy_star(struct tr_object * obj);
static void trm_set_accuracy_star(struct tr_map * map);

//-----------------------------------------------------

#define ACCURACY_FILE  "accuracy_cst.yaml"

#define MS_GREAT       48
#define MS_COEFF_GREAT 3
#define MS_GOOD        108
#define MS_COEFF_GOOD  6
#define MS_MISS        500 // arbitrary
#define MS_COEFF_MISS  0   // arbitrary

static struct vector * SLOW_VECT;
static struct vector * HIT_WINDOW_VECT;
static struct vector * SPC_FREQ_VECT;
static struct vector * SPC_INFLU_VECT;

static double ACCURACY_STAR_COEFF_SLOW;
static double ACCURACY_STAR_COEFF_HIT_WINDOW;
static double ACCURACY_STAR_COEFF_SPACING;
static struct vector * SCALE_VECT;

//-----------------------------------------------------

static void global_init(void)
{
    SLOW_VECT       = cst_vect(ht_cst, "vect_slow");
    HIT_WINDOW_VECT = cst_vect(ht_cst, "vect_hit_window");
    SPC_FREQ_VECT   = cst_vect(ht_cst, "vect_spacing_frequency");
    SPC_INFLU_VECT  = cst_vect(ht_cst, "vect_spacing_influence");
    SCALE_VECT      = cst_vect(ht_cst, "vect_scale");

    ACCURACY_STAR_COEFF_SLOW       = cst_f(ht_cst, "star_slow");
    ACCURACY_STAR_COEFF_HIT_WINDOW = cst_f(ht_cst, "star_hit_window");
    ACCURACY_STAR_COEFF_SPACING    = cst_f(ht_cst, "star_spacing");
}

//-----------------------------------------------------

__attribute__((constructor))
static void ht_cst_init_accuracy(void)
{
    yw = cst_get_yw(ACCURACY_FILE);
    ht_cst = cst_get_ht(yw);
    if(ht_cst)
	global_init();
}

__attribute__((destructor))
static void ht_cst_exit_accuracy(void)
{
    yaml2_free(yw);
    vect_free(SLOW_VECT);
    vect_free(SPC_FREQ_VECT);
    vect_free(SPC_INFLU_VECT);
    vect_free(HIT_WINDOW_VECT);
    vect_free(SCALE_VECT);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_slow(struct tr_object * obj)
{
    return vect_poly2(SLOW_VECT, obj->bpm_app);
}

//-----------------------------------------------------

static void tro_set_slow(struct tr_object * obj)
{
    if(obj->ps == MISS) {
	obj->slow = 0;
	return;
    }
    obj->slow = tro_slow(obj);
}

//-----------------------------------------------------

static void tro_set_hit_window(struct tr_object * obj, int ggm_ms[])
{
    switch(obj->ps) {
    case GREAT:
	obj->hit_window = ggm_ms[0];
	break;
    case GOOD:
	obj->hit_window = ggm_ms[1];
	break;
    default:
	obj->hit_window = ggm_ms[2];
	break;
    }
    obj->hit_window = vect_exp(HIT_WINDOW_VECT, obj->hit_window);
}

//-----------------------------------------------------

static int equal_i(int x, int y)
{
    return abs(x - y) < TIME_EQUAL_MS;
}

//-----------------------------------------------------

static double tro_spacing_influence(struct tr_object * o1, 
				    struct tr_object * o2)
{
    int diff = o2->offset - o1->offset;
    return vect_exp(SPC_INFLU_VECT, diff);
}

//-----------------------------------------------------

static struct list * tro_spacing_init(struct tr_object * objs, int i)
{
    struct tr_object * copy = tro_copy(objs, i+1);
    tro_sort_rest(copy, i+1);

    struct list * l = spc_new();
    int last = 0;
    spc_add_f(l, copy[last].rest,
	      tro_spacing_influence(&copy[last], &objs[i]));

    for(int j = 1; j < i+1; j++) {
	if(copy[j].ps == MISS)
	    continue;
	if(equal_i(copy[last].rest, copy[j].rest))
	    spc_increase_f(l, copy[last].rest, 
			   tro_spacing_influence(&copy[j], &objs[i]));
	else {
	    spc_add_f(l, copy[j].rest, 
		      tro_spacing_influence(&copy[j], &objs[i]));
	    last = j;
	}
    }
    //spc_print(l);
    free(copy);
    return l;
}

//-----------------------------------------------------

static double tro_spacing(struct tr_object * obj, struct list * spc)
{
    double freq = (spc_get_nb(spc, obj->rest, equal_i) / 
		   spc_get_total(spc));
    return vect_poly2(SPC_FREQ_VECT, freq);;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_set_hit_window(struct tr_map * map)
{
    int ggm_ms[3];
    ggm_ms[0] = (int)(map->od_mod_mult *
		      (MS_GREAT - (MS_COEFF_GREAT * map->od)));
    ggm_ms[1] = (int)(map->od_mod_mult *
		      (MS_GOOD  - (MS_COEFF_GOOD  * map->od)));
    ggm_ms[2] = (int)(map->od_mod_mult *
		      (MS_MISS  - (MS_COEFF_MISS  * map->od)));
    for(int i = 0; i < map->nb_object; i++)
	tro_set_hit_window(&map->object[i], ggm_ms);
}

//-----------------------------------------------------

static void trm_set_slow(struct tr_map * map)
{
    for (int i = 0; i < map->nb_object; i++)
	tro_set_slow(&map->object[i]);
}

//-----------------------------------------------------

static void tro_set_spacing(struct tr_object * objs, int i)
{
    struct list * l = tro_spacing_init(objs, i);
    objs[i].spacing = tro_spacing(&objs[i], l);
    spc_free(l);
}

static void trm_set_spacing(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	tro_set_spacing(map->object, i);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void tro_set_accuracy_star(struct tr_object * obj)
{
    obj->accuracy_star = vect_poly2
	(SCALE_VECT,
	 (ACCURACY_STAR_COEFF_SLOW       * obj->slow +
	  ACCURACY_STAR_COEFF_SPACING    * obj->spacing +
	  ACCURACY_STAR_COEFF_HIT_WINDOW * obj->hit_window));
}

//-----------------------------------------------------

static void trm_set_accuracy_star(struct tr_map * map)
{
    for (int i = 0; i < map->nb_object; i++)
	tro_set_accuracy_star(&map->object[i]);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_accuracy(struct tr_map * map)
{
    if(!ht_cst) {
	tr_error("Unable to compute accuracy stars.");
	return;
    }

    trm_set_hit_window(map);
    trm_set_spacing(map);
    trm_set_slow(map);
    trm_set_accuracy_star(map);
}
