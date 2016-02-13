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
#include "vector.h"
#include "print.h"

#include "pattern.h"

struct pattern
{
    double * d;
};

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;
static struct hash_table * ht_pattern;
static int pattern_set;

//--------------------------------------------------

static void remove_pattern(const char * s, void * p, void * null);
static void ht_pattern_init();

static double tro_singletap_proba(struct tr_object * obj);
static struct pattern * trm_get_pattern(struct tr_map * map,
					int i, double proba_alt);

static void trm_pattern_alloc(struct tr_map * map);
static void trm_compute_pattern_proba(struct tr_map * map);
static void trm_compute_pattern_alt(struct tr_map * map);

static void trm_compute_pattern_star(struct tr_map * map);

//--------------------------------------------------

#define PATTERN_FILE  "pattern_cst.yaml"

// coeff for singletap proba
static struct vector * SINGLETAP_VECT;

static int PROBA_START;
static int PROBA_END;
static int PROBA_STEP;

// coeff for star
static double PATTERN_STAR_COEFF_ALT;
static struct vector * SCALE_VECT;

// pattern
static int MAX_PATTERN_LENGTH;
static int LENGTH_PATTERN_USED;

#define cst_assert(COND, MSG)			\
    if(!(COND)) {				\
	tr_error(MSG);				\
	pattern_set = 0;			\
	return;					\
    }

//-----------------------------------------------------

static void global_init(void)
{
    SINGLETAP_VECT = cst_vect(ht_cst, "vect_singletap");
    SCALE_VECT     = cst_vect(ht_cst, "vect_scale");

    PROBA_START = cst_f(ht_cst, "proba_start");
    PROBA_END   = cst_f(ht_cst, "proba_end");
    PROBA_STEP  = cst_f(ht_cst, "proba_step");

    MAX_PATTERN_LENGTH = cst_i(ht_cst, "max_pattern_length");
    LENGTH_PATTERN_USED = cst_i(ht_cst, "length_pattern_used");
    
    PATTERN_STAR_COEFF_ALT = cst_f(ht_cst, "star_alt");
}

//-----------------------------------------------------

__attribute__((constructor))
static void ht_cst_init_pattern(void)
{
    yw = cst_get_yw(PATTERN_FILE);
    ht_cst = cst_get_ht(yw);
    if(ht_cst) {
	global_init();
	pattern_set = 1;
	ht_pattern_init();
    }
    else
	pattern_set = 0;
}

//-----------------------------------------------------

static void ht_pattern_init(void)
{  
    ht_pattern = ht_create(0, NULL);

    struct yaml_wrap * yw_l = NULL;
    ht_get_entry(ht_cst, "patterns", &yw_l);
    cst_assert(yw_l != NULL, "Pattern list not found.");
    cst_assert(yw_l->type == YAML_SEQUENCE,
	       "Pattern list is not a list.");
  
    struct list * pattern_l = yw_l->content.sequence;
    for(unsigned int i = 1; i <= list_size(pattern_l); i++) {
	struct yaml_wrap * yw_subl = list_get(pattern_l, i);
	cst_assert(yw_subl->type == YAML_SEQUENCE,
		   "Pattern structure is not a list.");
	struct list * pattern_data = yw_subl->content.sequence;

	cst_assert((unsigned int) LENGTH_PATTERN_USED + 1 == 
		   list_size(pattern_data),
		   "Pattern structure does not have the right size.");
      
	struct yaml_wrap * yw_name = list_get(pattern_data, 1);
	cst_assert(yw_name->type == YAML_SCALAR,
		   "Pattern name is not a scalar.");
	char * name = yw_name->content.scalar;

	struct pattern * p = calloc(sizeof(*p), 1);
	p->d = calloc(sizeof(double), LENGTH_PATTERN_USED);
      
	for(unsigned int j = 2; j <= list_size(pattern_data); j++) {
	    struct yaml_wrap * yw_s = list_get(pattern_data, j);
	    cst_assert(yw_s->type == YAML_SCALAR,
		       "Pattern value is not a scalar.");
	    p->d[j-2] = atof(yw_s->content.scalar);
	}
	ht_add_entry(ht_pattern, name, p);
    }
}
  
//-----------------------------------------------------

static void remove_pattern(const char * s __attribute__((unused)),
			   void * p,
			   void * null  __attribute__((unused)))
{
    free(((struct pattern *) p)->d);
    free(p);
}

//--------------------------------------------------

__attribute__((destructor))
static void ht_cst_exit_pattern(void)
{
    yaml2_free(yw);
    ht_for_each(ht_pattern, remove_pattern, NULL);
    ht_free(ht_pattern);
    vect_free(SCALE_VECT);
    vect_free(SINGLETAP_VECT);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_singletap_proba(struct tr_object * obj)
{
    return vect_poly2(SINGLETAP_VECT, obj->rest);
}

//-----------------------------------------------------

static struct pattern * trm_get_pattern(struct tr_map * map,
					int i, double proba_alt)
{
    char * s = calloc(sizeof(char), MAX_PATTERN_LENGTH + 1);
    for (int j = 0; (j < MAX_PATTERN_LENGTH &&
		     i + j < map->nb_object); j++) {
	if (tro_is_bonus(&map->object[i+j]) ||
	    map->object[i+j].ps == MISS) {
	    s[j] = 0;
	    break; // continue ? for bonus ?
	}
	else if (tro_is_don(&map->object[i+j]))
	    s[j] = 'd';
	else // if (tro_is_kat(&map->object[i+j]))
	    s[j] = 'k';
      
	if (tro_is_big(&map->object[i+j]) ||
	    map->object[i+j].proba > proba_alt) {
	    s[j+1] = 0;
	    break;
	}
    }

    struct pattern * p = NULL;
    int ret = ht_get_entry(ht_pattern, s, &p);
    if (ret != 0) { // when s[0] = 0 (bonus) or not found (error)
	if (s[0] != 0)
	    tr_error("Could not find pattern :%s", s);
	p = NULL;
    }
    free(s);
    return p;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_pattern_proba(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++) {
	map->object[i].proba = tro_singletap_proba(&map->object[i]);
    }
}

//-----------------------------------------------------

static void trm_compute_pattern_alt(struct tr_map * map)
{
    for(int pro = PROBA_START; pro <= PROBA_END; pro += PROBA_STEP) {
	double pro_p = pro / 100.;
	for(int i = 0; i < map->nb_object; i++) {
	    struct pattern * p = trm_get_pattern(map, i, pro_p);
	    if (p == NULL)
		continue;
	  
	    for (int j = 0; (j < LENGTH_PATTERN_USED &&
			     i + j < map->nb_object); j++)
		map->object[i+j].alt[j] += pro_p * p->d[j];
	}
    }
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_pattern_star(struct tr_map * map)
{
    for (int i = 0; i < map->nb_object; i++) {
	map->object[i].pattern_star = 0;
	for (int j = 0; j < LENGTH_PATTERN_USED; j++) {
	    map->object[i].pattern_star += vect_poly2
		(SCALE_VECT,
		 PATTERN_STAR_COEFF_ALT * map->object[i].alt[j]);
	}
    }
    map->pattern_star = trm_weight_sum_pattern_star(map, NULL);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_pattern_alloc(struct tr_map * map)
{
    for (int i = 0; i < map->nb_object; i++) { 
	// end with negative value
	map->object[i].alt =
	    calloc(sizeof(double), LENGTH_PATTERN_USED+1);
	map->object[i].alt[LENGTH_PATTERN_USED] = -1;
    }  
}

//-----------------------------------------------------

void trm_compute_pattern(struct tr_map * map)
{
    if(!pattern_set) {
	tr_error("Unable to compute pattern stars.");
	return;
    }
  
    trm_pattern_alloc(map);
    trm_compute_pattern_proba(map);
    trm_compute_pattern_alt(map);
  
    trm_compute_pattern_star(map);
}
