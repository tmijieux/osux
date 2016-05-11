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

#include "accuracy.h"
#include "spacing_count.h"

#define TIME_EQUAL_MS 12

static struct yaml_wrap * yw_acc;
static struct hash_table * ht_cst_acc;

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

static struct linear_fun * SLOW_VECT;
static struct linear_fun * HIT_WINDOW_VECT;
static struct linear_fun * SPC_FREQ_VECT;
static struct linear_fun * SPC_INFLU_VECT;

static double ACCURACY_STAR_COEFF_SLOW;
static double ACCURACY_STAR_COEFF_HIT_WINDOW;
static double ACCURACY_STAR_COEFF_SPACING;
static struct linear_fun * ACCURACY_SCALE_VECT;

//-----------------------------------------------------

static void accuracy_global_init(struct hash_table * ht_cst)
{
    SLOW_VECT       = cst_lf(ht_cst, "vect_slow");
    HIT_WINDOW_VECT = cst_lf(ht_cst, "vect_hit_window");
    SPC_FREQ_VECT   = cst_lf(ht_cst, "vect_spacing_frequency");
    SPC_INFLU_VECT  = cst_lf(ht_cst, "vect_spacing_influence");
    ACCURACY_SCALE_VECT = cst_lf(ht_cst, "vect_scale");

    ACCURACY_STAR_COEFF_SLOW       = cst_f(ht_cst, "star_slow");
    ACCURACY_STAR_COEFF_HIT_WINDOW = cst_f(ht_cst, "star_hit_window");
    ACCURACY_STAR_COEFF_SPACING    = cst_f(ht_cst, "star_spacing");
}

//-----------------------------------------------------
static void ht_cst_exit_accuracy(void)
{
    yaml2_free(yw_acc);
    lf_free(SLOW_VECT);
    lf_free(SPC_FREQ_VECT);
    lf_free(SPC_INFLU_VECT);
    lf_free(HIT_WINDOW_VECT);
    lf_free(ACCURACY_SCALE_VECT);
}

INITIALIZER(ht_cst_init_accuracy)
{
    yw_acc = cst_get_yw(ACCURACY_FILE);
    ht_cst_acc = yw_extract_ht(yw_acc);
    if (ht_cst_acc != NULL)
	accuracy_global_init(ht_cst_acc);
    atexit(ht_cst_exit_accuracy);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_slow(struct tr_object * obj)
{
    return lf_eval(SLOW_VECT, obj->bpm_app);
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
    default: // MISS & bonus
	obj->hit_window = ggm_ms[2];
	break;
    }
    obj->hit_window = lf_eval(HIT_WINDOW_VECT, obj->hit_window);
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
    return lf_eval(SPC_INFLU_VECT, diff);
}

//-----------------------------------------------------

static struct spacing_count * tro_spacing_init(struct tr_object * objs, int i)
{
    struct spacing_count * spc = spc_new(equal_i);
    for(int j = i; j >= 0; j--) {
	if(objs[j].ps == MISS)
	    continue;
	double influ = tro_spacing_influence(&objs[j], &objs[i]);
	if (influ == 0)
	    break; /* j-- influence won't increase */
	spc_add(spc, objs[j].rest, influ);
    }
    return spc;
}

//-----------------------------------------------------

static double tro_spacing(struct tr_object * obj,
			  struct spacing_count * spc)
{
    double total = spc_get_total(spc);
    double nb = spc_get_nb(spc, obj->rest);
    double freq = 1;
    if (total != 0) // avoid error when only miss
	freq = nb / total;
    return lf_eval(SPC_FREQ_VECT, freq);
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
    struct spacing_count * spc = tro_spacing_init(objs, i);
    objs[i].spacing = tro_spacing(&objs[i], spc);
    spc_free(spc);
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
    obj->accuracy_star = lf_eval
	(ACCURACY_SCALE_VECT,
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
    if(ht_cst_acc == NULL) {
	tr_error("Unable to compute accuracy stars.");
	return;
    }

    trm_set_hit_window(map);
    trm_set_spacing(map);
    trm_set_slow(map);
    trm_set_accuracy_star(map);
}
