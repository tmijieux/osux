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

#include "freq_counter.h"
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
    int len;
    double proba_alt;
};

//--------------------------------------------------

static double tro_singletap_proba(struct tr_object * obj);
static void pattern_free(struct pattern *p);

static void trm_set_patterns(struct tr_map * map);
static char * trm_extract_pattern_str(struct tr_map * map,
				      int i, double proba_alt);
static struct pattern * trm_extract_pattern(struct tr_map * map,
					    int i, double proba_alt);

static void tro_free_patterns(struct tr_object * o);
static void trm_free_patterns(struct tr_map * map);

static void tro_set_pattern_freq(struct tr_object * objs, int i);
static void trm_set_pattern_freq(struct tr_map * map);

static void tro_set_pattern_proba(struct tr_object * obj);
static void trm_set_pattern_proba(struct tr_map * map);

static void tro_set_pattern_star(struct tr_object * obj);
static void trm_set_pattern_star(struct tr_map * map);

//--------------------------------------------------

#define PATTERN_FILE  "pattern_cst.yaml"

// coeff for singletap proba
static struct linear_fun * SINGLETAP_VECT;
static struct linear_fun * PATTERN_VECT;
static struct linear_fun * INFLU_VECT;

static int PROBA_START;
static int PROBA_END;
static int PROBA_STEP;
static int PROBA_NB;

// coeff for star
static double PATTERN_STAR_COEFF_PATTERN;
static struct linear_fun * SCALE_VECT;

// pattern
static int MAX_PATTERN_LENGTH;

//-----------------------------------------------------

static void global_init(void)
{
    PATTERN_VECT   = cst_lf(ht_cst, "vect_pattern");
    INFLU_VECT     = cst_lf(ht_cst, "vect_influence");
    SINGLETAP_VECT = cst_lf(ht_cst, "vect_singletap");
    SCALE_VECT     = cst_lf(ht_cst, "vect_scale");

    PROBA_START = cst_f(ht_cst, "proba_start");
    PROBA_END   = cst_f(ht_cst, "proba_end");
    PROBA_STEP  = cst_f(ht_cst, "proba_step");
    PROBA_NB    = (PROBA_END - PROBA_START) / PROBA_STEP + 1;

    MAX_PATTERN_LENGTH = cst_i(ht_cst, "max_pattern_length");
    
    PATTERN_STAR_COEFF_PATTERN = cst_f(ht_cst, "star_pattern");
}

//-----------------------------------------------------

__attribute__((constructor))
static void ht_cst_init_pattern(void)
{
    yw = cst_get_yw(PATTERN_FILE);
    ht_cst = yw_extract_ht(yw);
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
/*
// test is pattern are the same or if one is include in the other
// dddk & dddk -> 1
// dddk & ddd  -> 1
// ddkk & ddd  -> 0
static int pattern_is_in(struct pattern * p1, struct pattern * p2)
{
    int i = 0;
    while (p1->s[i] && p2->s[i])
	if (p1->s[i] != p2->s[i])
	    return 0;
    return 1;
}
*/
//-----------------------------------------------------

static void trm_set_patterns(struct tr_map * map)
{   
    for(int i = 0; i < map->nb_object; i++) {
	struct tr_object * o = &map->object[i];
	o->patterns = malloc(sizeof(struct pattern*) * PROBA_NB);
	int j = 0;
	for(int p = PROBA_START; p <= PROBA_END; p+=PROBA_STEP) {
	    o->patterns[j] = trm_extract_pattern(map, i, 
						 p / PROBA_SCALE);
	    j++;
	}
    }
}

//-----------------------------------------------------

static struct pattern * trm_extract_pattern(struct tr_map * map,
					    int i, double proba_alt)
{
    struct pattern * p = malloc(sizeof(*p));
    p->proba_alt = proba_alt;
    p->s = trm_extract_pattern_str(map, i, proba_alt);
    p->len = strlen(p->s);
    return p;
}

//-----------------------------------------------------

static char * trm_extract_pattern_str(struct tr_map * map,
				      int i, double proba_alt)
{
    char * s = calloc(sizeof(char), MAX_PATTERN_LENGTH + 1);
    for (int j = 0; j < MAX_PATTERN_LENGTH && i + j < map->nb_object; j++) {
	if (tro_is_bonus(&map->object[i+j]) ||
	    map->object[i+j].ps == MISS) {
	    s[j] = 0;
	    break; // continue?
	}
	else if (tro_is_don(&map->object[i+j]))
	    s[j] = 'd';
	else
	    s[j] = 'k';
	
	if ((!(i + j + 1 < map->nb_object)) || 
	    proba_alt < map->object[i+j+1].proba) {
	    s[j+1] = 0;
	    break;
	}
    }
    return s;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_pattern_influence(struct tr_object * o1,
				    struct tr_object * o2)
{
    int diff = o2->offset - o1->offset;
    return lf_eval(INFLU_VECT, diff);
}

//-----------------------------------------------------

static double tro_pattern_freq(struct tr_object * o,
			       struct counter * c)
{
    double total = cnt_get_total(c) * PROBA_NB;
    double nb = 0;
    for (int k = 0; k < PROBA_NB; k++)
	nb += cnt_get_nb(c, o->patterns[k]->s);
    return nb / total;
}

//-----------------------------------------------------

static struct counter * tro_pattern_freq_init(struct tr_object *objs,
					      int i)
{
    struct counter * c = cnt_new();
    for (int j = 0; j <= i; j++) {
	for (int k = 0; k < PROBA_NB; k++) {
	    if (objs[j].patterns[k]->s[0] == '\0')
		continue;
	    cnt_add(c, objs[j].patterns[k], objs[j].patterns[k]->s,
		    tro_pattern_influence(&objs[j], &objs[i]));
	}
    }
    return c;
}

//-----------------------------------------------------

static double tro_singletap_proba(struct tr_object * obj)
{
    return lf_eval(SINGLETAP_VECT, obj->rest);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void tro_set_pattern_freq(struct tr_object * objs, int i)
{
    struct counter * c = tro_pattern_freq_init(objs, i);

    double freq = tro_pattern_freq(&objs[i], c);
    objs[i].pattern = lf_eval(PATTERN_VECT, freq);

    for(int j = 0; j < PROBA_NB; j++) {
	printf("%s\t%g\n", objs[i].patterns[j]->s, 
	       objs[i].patterns[j]->proba_alt);
    }
    cnt_print(c);
    printf("--------------------------\n");

    cnt_free(c);
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
    for(int i = 0; i < map->nb_object; i++)
	tro_set_pattern_freq(map->object, i);
}

//-----------------------------------------------------

static void trm_set_pattern_proba(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	tro_set_pattern_proba(&map->object[i]);
}

//-----------------------------------------------------

static void pattern_free(struct pattern *p)
{
    if (p == NULL)
	return;
    free(p->s);
    free(p);
}

static void tro_free_patterns(struct tr_object * o)
{    
    for(int i = 0; i < PROBA_NB; i++)
	pattern_free(o->patterns[i]);
    free(o->patterns);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void tro_set_pattern_star(struct tr_object * obj)
{
    obj->pattern_star = lf_eval
	(SCALE_VECT, PATTERN_STAR_COEFF_PATTERN * obj->pattern);
}

//-----------------------------------------------------

static void trm_set_pattern_star(struct tr_map * map)
{
    for (int i = 0; i < map->nb_object; i++)
	tro_set_pattern_star(&map->object[i]);
}

//-----------------------------------------------------

static void trm_free_patterns(struct tr_map * map)
{
    for (int i = 0; i < map->nb_object; i++)
	tro_free_patterns(&map->object[i]);
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
    trm_set_patterns(map);
    trm_set_pattern_freq(map);
    trm_free_patterns(map);
  
    trm_set_pattern_star(map);
}
