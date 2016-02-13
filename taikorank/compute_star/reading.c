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

#include <time.h>
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
#include "vector.h"
#include "print.h"

#include "reading.h"

#define min(x, y) x < y ? x : y;
#define max(x, y) x > y ? x : y;
#define RAND_DOUBLE ((double) rand() / RAND_MAX)

typedef int (*mc_cond)(double, double, void *);

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;

static int pt_is_in_tro(double x, double y, struct tr_object * o);
static int pt_is_in_intersection(double x, double y,
				 struct tro_table * t);

static double tr_monte_carlo(int nb_pts,
			     double x1, double y1,
			     double x2, double y2,
			     int (*is_in)(double, double, void *), 
			     void * arg);

static int tro_same_bpm_hide(struct tr_object * o, 
			     struct tro_table * obj_h);
static double tro_seen_area(struct tr_object * o);
static double tro_hide(struct tr_object *o, struct tro_table *obj_h);
static double tro_seen(struct tr_object *o, struct tro_table *obj_h);
static struct tro_table * tro_get_obj_hiding(struct tr_object * objs,
					     int i);

static void tro_set_seen(struct tr_object * objs, int i);
static void trm_set_seen(struct tr_map * map);

static void tro_set_reading_star(struct tr_object * obj);
static void trm_set_reading_star(struct tr_map * map);

//--------------------------------------------------

#define READING_FILE  "reading_cst.yaml"

static int MONTE_CARLO_NB_PT;
static struct vector * SEEN_VECT;

// coeff for star
static double READING_STAR_COEFF_SEEN;
static struct vector * SCALE_VECT;

//-----------------------------------------------------

static void global_init(void)
{
    srand(time(NULL));
    MONTE_CARLO_NB_PT = cst_i(ht_cst, "monte_carlo_nb_pts");

    SEEN_VECT  = cst_vect(ht_cst, "vect_seen");
    SCALE_VECT = cst_vect(ht_cst, "vect_scale");

    READING_STAR_COEFF_SEEN       = cst_f(ht_cst, "star_seen");
}

//-----------------------------------------------------

__attribute__((constructor))
static void ht_cst_init_reading(void)
{
    yw = cst_get_yw(READING_FILE);
    ht_cst = cst_get_ht(yw);
    if(ht_cst)
	global_init();
}

__attribute__((destructor))
static void ht_cst_exit_reading(void)
{
    yaml2_free(yw);
    vect_free(SEEN_VECT);
    vect_free(SCALE_VECT);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static int pt_is_in_tro(double x, double y, struct tr_object * o)
{
    // y(x) = bpm_app * x + c_app
    // y(x) = bpm_app * x + c_end_app
    return (o->bpm_app * x + o->c_end_app <= y &&
	    o->bpm_app * x + o->c_app     >= y);
}

static int pt_is_in_intersection(double x, double y,
				 struct tro_table * t)
{
    for(int i = 0; i < t->l; i++) {
	if(t->t[i] == NULL)
	    continue;
	if(!pt_is_in_tro(x, y, t->t[i]))
	    return 0;
    }
    return 1;
}

//-----------------------------------------------------

static double tr_monte_carlo(int nb_pts,
			     double x1, double y1,
			     double x2, double y2,
			     int (*is_in)(double, double, void *), 
			     void * arg)
{
    int ok = 0;
    for(int i = 0; i < nb_pts; i++) {
	double x = x1 + RAND_DOUBLE * (x2 - x1); // x1 <= x <= x2
	double y = y1 + RAND_DOUBLE * (y2 - y1); // y1 <= y <= y2
	if(is_in(x, y, arg))
	    ok++;
    }
    return ((fabs(x1 - x2) * fabs(y1 - y2)) *
	    ((double) ok / (double) nb_pts));
}

//-----------------------------------------------------

/**
 * Change 'o' offset according to same bpm object that are hiding it.
 * And set them to NULL
 * @return Number of object with the same bpm  
 */
static int tro_same_bpm_hide(struct tr_object * o, 
			     struct tro_table * obj_h)
{
    int done = 0;
    for(int i = 0; i < obj_h->l; i++) {
	if(equal(o->bpm_app, obj_h->t[i]->bpm_app)) {
	    if(o->offset_app < obj_h->t[i]->offset_dis) {
		o->offset_app     = obj_h->t[i]->offset_dis;
		o->end_offset_app = obj_h->t[i]->end_offset_dis;
	    }
	    obj_h->t[i] = NULL;
	    done++;
	}
    }
    return done;
}

//-----------------------------------------------------

/**
 * @return visible (surface * time) if the object was not hidden
 */
static double tro_seen_area(struct tr_object * o)
{
    double seen = 
	((o->end_offset_dis - o->offset_app) * 
	 (o->bpm_app * o->end_offset_dis + o->c_app)
	 -
	 (o->end_offset_dis - o->end_offset_app) * 
	 (o->bpm_app * o->end_offset_dis + o->c_app));
    seen *= tro_get_size(o);
    return seen;
}

//-----------------------------------------------------

/**
 * @return (surface * time) hidden by obj_h
 */
static double tro_hide(struct tr_object *o, struct tro_table *obj_h)
{
    double x1 = o->offset_app;
    double x2 = o->end_offset_dis;
    double y1 = 0;
    double y2 = o->bpm_app * o->end_offset_dis + o->c_app;
    return tr_monte_carlo(MONTE_CARLO_NB_PT, x1, y1, x2, y2, 
			  (mc_cond)pt_is_in_intersection, 
			  obj_h);
}

//-----------------------------------------------------

static double tro_seen(struct tr_object *o, struct tro_table *obj_h)
{
    struct tr_object * copy = tro_copy(o, 1);

    int done = tro_same_bpm_hide(copy, obj_h);
    double seen = tro_seen_area(copy);
    if(done != obj_h->l) {
	seen -= tro_hide(copy, obj_h);
    }

    free(copy);
    return vect_poly2(SEEN_VECT, seen);
}

//-----------------------------------------------------

static struct tro_table * tro_get_obj_hiding(struct tr_object * objs,
					     int i)
{
    // list object that hide the i-th
    struct tro_table * obj_h = tro_table_new(i);

    for(int j = 0; j < i; j++) { 
	if(objs[j].ps == MISS)
	    continue;
	// if i has appeared before j
	if(objs[j].end_offset_app - objs[i].offset_app > 0)
	    tro_table_add(obj_h, &objs[j]);
    }
    return obj_h;
}

//-----------------------------------------------------

static void tro_set_seen(struct tr_object * objs, int i)
{
    if(objs[i].ps == MISS) {
	objs[i].seen = 0;
	return;
    }

    struct tro_table * obj_h = tro_get_obj_hiding(objs, i);
    objs[i].seen = tro_seen(&objs[i], obj_h);
    tro_table_free(obj_h);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_set_seen(struct tr_map * map)
{
    for (int i = 0; i < map->nb_object; i++)
	tro_set_seen(map->object, i);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void tro_set_reading_star(struct tr_object * obj)
{
    obj->reading_star = vect_poly2
	(SCALE_VECT, READING_STAR_COEFF_SEEN * obj->seen);
}

//-----------------------------------------------------

static void trm_set_reading_star(struct tr_map * map)
{
    for (int i = 0; i < map->nb_object; i++)
	tro_set_reading_star(&map->object[i]);
    map->reading_star = trm_weight_sum_reading_star(map, NULL);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_reading(struct tr_map * map)
{
    if(!ht_cst) {
	tr_error("Unable to compute reading stars.");
	return;
    }
  
    trm_set_seen(map);
    trm_set_reading_star(map);
}
