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

#include "pattern.h"

#define PROBA_SCALE 100.

static struct yaml_wrap * yw;
static struct hash_table * ht_cst;

struct pattern {
    char * s;
    int offset;
    double proba_alt;
};

struct pattern_table {
    struct pattern ** p;
    int size;
};

//--------------------------------------------------

static struct pattern_table * trm_init_patterns(struct tr_map * map);
static struct pattern_table * pattern_table_new(int size);
static void pattern_table_free(struct pattern_table * p);
static void pattern_free(struct pattern *p);

static char * trm_extract_pattern_str(struct tr_map * map,
				      int i, double proba_alt);
static struct pattern * trm_extract_pattern(struct tr_map * map,
					    int i, double proba_alt);

static double tro_singletap_proba(struct tr_object * obj);

static void tro_set_pattern_freq(struct tr_object * objs, int i,
				 struct pattern_table * p);
static void trm_set_pattern_freq(struct tr_map * map);

static void tro_set_pattern_proba(struct tr_object * obj);
static void trm_set_pattern_proba(struct tr_map * map);

static void tro_set_pattern_star(struct tr_object * obj);
static void trm_set_pattern_star(struct tr_map * map);

//--------------------------------------------------

#define PATTERN_FILE  "pattern_cst.yaml"

// coeff for singletap proba
static struct linear_fun * SINGLETAP_VECT;

static int PROBA_START;
static int PROBA_END;
static int PROBA_STEP;

// coeff for star
static double PATTERN_STAR_COEFF_ALT;
static struct linear_fun * SCALE_VECT;

// pattern
static int MAX_PATTERN_LENGTH;

//-----------------------------------------------------

static void global_init(void)
{
    SINGLETAP_VECT = cst_lf(ht_cst, "vect_singletap");
    SCALE_VECT     = cst_lf(ht_cst, "vect_scale");

    PROBA_START = cst_f(ht_cst, "proba_start");
    PROBA_END   = cst_f(ht_cst, "proba_end");
    PROBA_STEP  = cst_f(ht_cst, "proba_step");

    MAX_PATTERN_LENGTH = cst_i(ht_cst, "max_pattern_length");
    
    PATTERN_STAR_COEFF_ALT = cst_f(ht_cst, "star_alt");
}

//-----------------------------------------------------

__attribute__((constructor))
static void ht_cst_init_pattern(void)
{
    yw = cst_get_yw(PATTERN_FILE);
    ht_cst = cst_get_ht(yw);
    if(ht_cst != NULL)
	global_init();
}

//-----------------------------------------------------

__attribute__((destructor))
static void ht_cst_exit_pattern(void)
{
    yaml2_free(yw);
    lf_free(SCALE_VECT);
    lf_free(SINGLETAP_VECT);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void pattern_free(struct pattern *p)
{
    if (p == NULL)
	return;
    free(p->s);
    free(p);
}

static void pattern_table_free(struct pattern_table * p)
{
    if (p == NULL)
	return;
    for(int i = 0; i < p->size; i++)
	pattern_free(p->p[i]);
    free(p->p);
    free(p);
}

static struct pattern_table * pattern_table_new(int size)
{
    struct pattern_table * p = malloc(sizeof(*p));
    p->p = calloc(sizeof(struct pattern*), size);
    p->size = size;
    return p;
}

static struct pattern_table * trm_init_patterns(struct tr_map * map)
{
    int nb_proba = (PROBA_END - PROBA_START) / PROBA_STEP;
    struct pattern_table * p = pattern_table_new(map->nb_object * nb_proba);
    
    for(int i = 0; i < map->nb_object; i++) {
	int j = 0;
	for(double pr = PROBA_START; pr < PROBA_END; pr+=PROBA_STEP){
	    int k = j + nb_proba * i;
	    p->p[k] = trm_extract_pattern(map, i, pr / PROBA_SCALE);
	    j++;
	}
    }
    return p;
}

//-----------------------------------------------------

static struct pattern * trm_extract_pattern(struct tr_map * map,
					    int i, double proba_alt)
{
    struct pattern * p = malloc(sizeof(*p));
    p->proba_alt = proba_alt;
    p->offset = map->object[i].offset;
    p->s = trm_extract_pattern_str(map, i, proba_alt);
    return p;
}

//-----------------------------------------------------

static char * trm_extract_pattern_str(struct tr_map * map,
				      int i, double proba_alt)
{
    char * s = calloc(sizeof(char), MAX_PATTERN_LENGTH + 1);
    for (int j = 0; 
	 j < MAX_PATTERN_LENGTH && i + j < map->nb_object; 
	 j++) {
	if (tro_is_bonus(&map->object[i+j]) ||
	    map->object[i+j].ps == MISS) {
	    s[j] = 0;
	    break; // continue?
	}
	else if (tro_is_don(&map->object[i+j]))
	    s[j] = 'd';
	else // if (tro_is_kat(&map->object[i+j]))
	    s[j] = 'k';
	
	if (tro_is_big(&map->object[i+j]) ||
	    i + j > map->nb_object || 
	    map->object[i+j+1].proba > proba_alt) {
	    s[j+1] = 0;
	    break;
	}
    }
    return s;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_singletap_proba(struct tr_object * obj)
{
    return lf_eval(SINGLETAP_VECT, obj->rest);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void tro_set_pattern_freq(struct tr_object * objs, int i,
				 struct pattern_table * p)
{
    for(int j = 0; j < p->size; j++) {
	if (p->p[i]->offset > objs[i].offset)
	    break; // array is sorted by offset
	;
    }
}

//-----------------------------------------------------

static void tro_set_pattern_proba(struct tr_object * obj)
{
    obj->proba = tro_singletap_proba(obj);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_set_pattern_freq(struct tr_map * map)
{
    struct pattern_table * p = trm_init_patterns(map);
/*
    printf("Pattern\n");
    for(int i = 0; i < p->size; i++) {
	if(p->p[i] == NULL)
	    continue;
	printf("%s\t%d\t%g\n", p->p[i]->s, p->p[i]->offset, p->p[i]->proba_alt);
    }
*/
    for(int i = 0; i < map->nb_object; i++)
	tro_set_pattern_freq(map->object, i, p);
    pattern_table_free(p);
}

//-----------------------------------------------------

static void trm_set_pattern_proba(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	tro_set_pattern_proba(&map->object[i]);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void tro_set_pattern_star(struct tr_object * obj)
{
    obj->pattern_star = 0;
}

//-----------------------------------------------------

static void trm_set_pattern_star(struct tr_map * map)
{
    for (int i = 0; i < map->nb_object; i++)
	tro_set_pattern_star(&map->object[i]);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_pattern(struct tr_map * map)
{
    if(ht_cst == NULL) {
	tr_error("Unable to compute pattern stars.");
	return;
    }
  
    trm_set_pattern_proba(map);
    trm_set_pattern_freq(map);
  
    trm_set_pattern_star(map);
}
