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
#include "util/table.h"
#include "compiler.h"

#include "freq_counter.h"
#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "stats.h"
#include "cst_yaml.h"
#include "linear_fun.h"
#include "print.h"

#include "pattern.h"

#define PROBA_SCALE 100.

static struct yaml_wrap * yw_ptr;
static struct hash_table * ht_cst_ptr;

struct pattern {
    char * s;
    double proba_start;
    double proba_end;
};

//--------------------------------------------------

static double tro_singletap_proba(struct tr_object * o1, 
				  struct tr_object * o2);
static void pattern_free(struct pattern *p);

static void trm_extract_pattern_str(struct tr_map * map,
				      int i, struct pattern * p);
static struct pattern * trm_extract_pattern(struct tr_map * map,
					    int i, double proba);

static void trm_set_pattern_proba(struct tr_map * map);
static void trm_set_type(struct tr_map * map);
static void trm_set_pattern_freq(struct tr_map * map);
static void trm_free_patterns(struct tr_map * map);
static void trm_set_pattern_star(struct tr_map * map);

//--------------------------------------------------

#define PATTERN_FILE  "pattern_cst.yaml"

// coeff for singletap proba
static struct linear_fun * SINGLETAP_VECT;
static struct linear_fun * PATTERN_FREQ_VECT;
static struct linear_fun * PATTERN_INFLU_VECT;

static double PROBA_START;
static double PROBA_END;

// coeff for star
static double PATTERN_STAR_COEFF_PATTERN;
static struct linear_fun * PATTERN_SCALE_VECT;

// pattern
static int MAX_PATTERN_LENGTH;


static inline void print_pattern(const struct pattern * p)
{
    fprintf(stderr, "%s\t%.4g\t%.4g\t(%.4g)\n", 
	    p->s, p->proba_start, p->proba_end,
	    p->proba_end - p->proba_start);
}

static inline void tro_print_pattern(const struct tr_object * o)
{
    for(int j = 0; j < table_len(o->patterns); j++)
	print_pattern(table_get(o->patterns, j));
}

//-----------------------------------------------------

static void pattern_global_init(struct hash_table * ht_cst)
{
    PATTERN_FREQ_VECT = cst_lf(ht_cst, "vect_pattern_freq");
    PATTERN_INFLU_VECT = cst_lf(ht_cst, "vect_influence");
    SINGLETAP_VECT = cst_lf(ht_cst, "vect_singletap");
    PATTERN_SCALE_VECT = cst_lf(ht_cst, "vect_scale");

    PROBA_START = (double)cst_i(ht_cst, "proba_start") / PROBA_SCALE;
    PROBA_END   = (double)cst_i(ht_cst, "proba_end")   / PROBA_SCALE;

    MAX_PATTERN_LENGTH = cst_i(ht_cst, "max_pattern_length");
    
    PATTERN_STAR_COEFF_PATTERN = cst_f(ht_cst, "star_pattern");
}


//-----------------------------------------------------

static void ht_cst_exit_pattern(void)
{
    yaml2_free(yw_ptr);
    lf_free(PATTERN_SCALE_VECT);
    lf_free(SINGLETAP_VECT);
    lf_free(PATTERN_FREQ_VECT);
    lf_free(PATTERN_INFLU_VECT);
}

INITIALIZER(ht_cst_init_pattern)
{
    yw_ptr = cst_get_yw(PATTERN_FILE);
    ht_cst_ptr = yw_extract_ht(yw_ptr);
    if (ht_cst_ptr != NULL)
	pattern_global_init(ht_cst_ptr);
    atexit(ht_cst_exit_pattern);
}


//-----------------------------------------------------
/*
static int pattern_1_is_in_2(struct pattern *p1, struct pattern *p2)
{
    int i;
    for (i = 0; p1->s[i] && p2->s[i]; i++) {
	if (p1->s[i] != p2->s[i])
	    return 0;
    }
    return p1->s[i] == '\0';
}
*/
static int pattern_is_in(struct pattern *p1, struct pattern *p2)
{
    for (int i = 0; p1->s[i] && p2->s[i]; i++) {
	if (p1->s[i] != p2->s[i])
	    return 0;
    }
    return 1;
}

//-----------------------------------------------------

void trm_set_patterns(struct tr_map * map)
{   
    for(int i = 0; i < map->nb_object; i++) {
	struct tr_object * o = &map->object[i];
	// taking one more space for some special case
	o->patterns = table_new(MAX_PATTERN_LENGTH + 1);
	double proba = PROBA_START;
	while (proba < PROBA_END) {
	    struct pattern * p = trm_extract_pattern(map, i, proba);
	    if (table_max(o->patterns) <= table_len(o->patterns)) {
		fprintf(stderr, "Out of bounds %d / %d on object n°%d (offset %d)\n", table_len(o->patterns), table_max(o->patterns), i, o->offset);
		print_pattern(p);
		tro_print_pattern(o);
		fprintf(stderr, "-----------------\n");
	    }
	    table_add(o->patterns, p);
	    proba = p->proba_end;
	}
    }
}

//-----------------------------------------------------

static struct pattern * trm_extract_pattern(struct tr_map * map,
					    int i, double proba)
{
    struct pattern * p = malloc(sizeof(*p));
    p->proba_start = proba;
    p->proba_end   = PROBA_END;
    trm_extract_pattern_str(map, i, p);
    return p;
}

//-----------------------------------------------------

static void trm_extract_pattern_str(struct tr_map * map,
				    int i, struct pattern * p)
{
    char * s = calloc(sizeof(char), MAX_PATTERN_LENGTH + 1);
    for (int j = 0; j < MAX_PATTERN_LENGTH && i + j < map->nb_object; j++) {
	s[j] = map->object[i+j].type;
	if (s[j] == '\0')
	    break;
	
	if (!(i + j + 1 < map->nb_object)) { // no more objects
	    break;
	}
	if (p->proba_start < map->object[i+j+1].proba) {
	    p->proba_end = map->object[i+j+1].proba;
	    break;
	}
    }
    p->s = s;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_pattern_influence(struct tr_object * o1,
				    struct tr_object * o2)
{
    int diff = o2->offset - o1->offset;
    return lf_eval(PATTERN_INFLU_VECT, diff);
}

//-----------------------------------------------------

static double tro_pattern_freq(struct tr_object * o,
			       struct counter * c)
{
    typedef int (*herit)(void*, void*);
    double total = cnt_get_total(c);
    if (total == 0)
	return 1;
    double nb = 0;
    for (int k = 0; k < table_len(o->patterns); k++) {
	struct pattern * p = table_get(o->patterns, k);
	double d = cnt_get_nb_compressed(c, p->s, (herit) pattern_is_in);
	nb += d * (p->proba_end - p->proba_start);
    }
    return nb / total;
}

//-----------------------------------------------------

static struct counter * tro_pattern_freq_init(struct tr_object *objs,
					      int i)
{
    struct counter * c = cnt_new();
    for (int j = i; j >= 0; j--) {
	double influ = tro_pattern_influence(&objs[j], &objs[i]);
	if (influ == 0)
	    break; // j-- influence will remain 0
	for (int k = 0; k < table_len(objs[j].patterns); k++) {
	    struct pattern * p = table_get(objs[j].patterns, k);
	    if (p->s[0] == '\0')
		continue;
	    cnt_add(c, p, p->s, influ);
	}
    }
    return c;
}

//-----------------------------------------------------

static double tro_singletap_proba(struct tr_object * o1, 
				  struct tr_object * o2)
{
    int diff = o2->offset - o1->end_offset;
    return lf_eval(SINGLETAP_VECT, diff);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void tro_set_pattern_freq(struct tr_object * objs, int i)
{
    struct counter * c = tro_pattern_freq_init(objs, i);

    double freq = tro_pattern_freq(&objs[i], c);
    objs[i].pattern = lf_eval(PATTERN_FREQ_VECT, freq);
/*
    tro_print_pattern(&objs[i]);
    typedef int (*herit)(void*, void*);
    cnt_print_compressed(c, (herit) pattern_is_in);
    printf("Pattern value for obj n°%d: %g\n", i, objs[i].pattern);
    printf("----------------------------------------------------\n");
*/
    cnt_free(c);
}

//-----------------------------------------------------

void tro_set_pattern_proba(struct tr_object * objs, int i)
{
    for (int j = i-1; j >= 0; j--) {
	if (tro_are_same_type(&objs[i], &objs[j])) {
	    objs[i].proba = tro_singletap_proba(&objs[j], &objs[i]);
	    return;
	}
    }
    objs[i].proba = 1;
}

//-----------------------------------------------------

void tro_set_type(struct tr_object * o)
{
    if (tro_is_bonus(o) || o->ps == MISS)
	o->type = '\0';
    else if (tro_is_don(o))
	o->type = 'd';
    else
	o->type = 'k';    
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_set_type(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	tro_set_type(&map->object[i]);    
}

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
	tro_set_pattern_proba(map->object, i);
}

//-----------------------------------------------------

static void pattern_free(struct pattern *p)
{
    if (p == NULL)
	return;
    free(p->s);
    free(p);
}

void tro_free_patterns(struct tr_object * o)
{
    for(int i = 0; i < table_len(o->patterns); i++)
	pattern_free(table_get(o->patterns, i));
    table_free(o->patterns);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void tro_set_pattern_star(struct tr_object * obj)
{
    obj->pattern_star = lf_eval
	(PATTERN_SCALE_VECT, 
	 PATTERN_STAR_COEFF_PATTERN * obj->pattern);
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
    if(ht_cst_ptr == NULL) {
	tr_error("Unable to compute pattern stars.");
	return;
    }
  
    trm_set_pattern_proba(map);
    trm_set_type(map);
    trm_set_patterns(map);
    trm_set_pattern_freq(map);
    trm_free_patterns(map);
  
    trm_set_pattern_star(map);
}
