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
static osux_hashtable * ht_cst_ptr;

struct pattern {
    char * s;
    int len;
    double proba_start;
    double proba_end;
    double proba_total;
};

//--------------------------------------------------

static struct pattern *
tro_extract_pattern(const struct tr_object * o, int i, int nb,
		    double proba);
static void tro_pattern_set_str(const struct tr_object * o,
				int i, int nb, struct pattern * p);

static double tro_pattern_influence(const struct tr_object * o1,
				    const struct tr_object * o2);

static struct counter * tro_pattern_new_counter(const struct tr_object * o, int i);
static double tro_pattern_freq(const struct tr_object * o,
			       const struct counter * c);

static void pattern_free(struct pattern *p);

static void trm_set_pattern_proba(struct tr_map * map);
static void trm_set_type(struct tr_map * map);
static void trm_set_pattern_freq(struct tr_map * map);
static void trm_free_patterns(struct tr_map * map);
static void trm_set_pattern_star(struct tr_map * map);

//--------------------------------------------------

#define PATTERN_FILE  "pattern_cst.yaml"

// coeff for singletap proba
static struct linear_fun * SINGLETAP_LF;
static struct linear_fun * PATTERN_FREQ_LF;
static struct linear_fun * PATTERN_INFLU_LF;

static double PROBA_START;
static double PROBA_END;

// coeff for star
static double PATTERN_STAR_COEFF_PATTERN;
static struct linear_fun * PATTERN_SCALE_LF;

// pattern
static int MAX_PATTERN_LENGTH;

//-----------------------------------------------------

static void pattern_global_init(osux_hashtable * ht_cst)
{
    PATTERN_FREQ_LF = cst_lf(ht_cst, "vect_pattern_freq");
    PATTERN_INFLU_LF = cst_lf(ht_cst, "vect_influence");
    SINGLETAP_LF = cst_lf(ht_cst, "vect_singletap");
    PATTERN_SCALE_LF = cst_lf(ht_cst, "vect_pattern_scale");

    PROBA_START = (double)cst_i(ht_cst, "proba_start") / PROBA_SCALE;
    PROBA_END   = (double)cst_i(ht_cst, "proba_end")   / PROBA_SCALE;

    MAX_PATTERN_LENGTH = cst_i(ht_cst, "max_pattern_length");

    PATTERN_STAR_COEFF_PATTERN = cst_f(ht_cst, "star_pattern");
}

//-----------------------------------------------------

static void ht_cst_exit_pattern(void)
{
    yaml2_free(yw_ptr);
    lf_free(PATTERN_SCALE_LF);
    lf_free(SINGLETAP_LF);
    lf_free(PATTERN_FREQ_LF);
    lf_free(PATTERN_INFLU_LF);
}

void tr_pattern_initialize(void)
{
    yw_ptr = cst_get_yw(PATTERN_FILE);
    ht_cst_ptr = yw_extract_ht(yw_ptr);
    if (ht_cst_ptr != NULL)
	pattern_global_init(ht_cst_ptr);
    atexit(ht_cst_exit_pattern);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static inline void print_pattern(const struct pattern * p)
{
    fprintf(stderr, "%s\t%.4g\t%.4g\t(%.4g)\n",
	    p->s, p->proba_start, p->proba_end, p->proba_total);
}

static inline void tro_print_pattern(const struct tr_object * o)
{
    for(int j = 0; j < table_len(o->patterns); j++)
	print_pattern(table_get(o->patterns, j));
}

//-----------------------------------------------------

__attribute__ ((unused))
static double pattern_are_same_length(const struct pattern *p1,
				      const struct pattern *p2)
{
    return p1->len == p2->len;
}

__attribute__ ((unused))
static double pattern_1_is_in_2(const struct pattern *p1,
				const struct pattern *p2)
{
    int i;
    for (i = 0; p1->s[i] && p2->s[i]; i++) {
	if (p1->s[i] != p2->s[i])
	    return 0;
    }
    return p1->s[i] == '\0';
}

__attribute__ ((unused))
static double pattern_is_in(const struct pattern *p1,
			    const struct pattern *p2)
{
    for (int i = 0; p1->s[i] && p2->s[i]; i++) {
	if (p1->s[i] != p2->s[i])
	    return 0;
    }
    return 1;
}

__attribute__ ((unused))
static double pattern_eq(const struct pattern *p1,
			 const struct pattern *p2)
{
    return strcmp(p1->s, p2->s) == 0;
}

__attribute__ ((unused))
static double pattern_1_common_begining(const struct pattern *p1,
					const struct pattern *p2)
{
    int i;
    for (i = 0; p1->s[i] && p2->s[i]; i++) {
	if (p1->s[i] != p2->s[i])
	    break;
    }
    return (double) i / strlen(p1->s);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void tro_pattern_set_str(const struct tr_object * o,
				int i, int nb, struct pattern * p)
{
    char * s = calloc(sizeof(char), MAX_PATTERN_LENGTH + 1);
    for (int j = 0; j < MAX_PATTERN_LENGTH && i + j < nb; j++) {
	s[j] = o->objs[i+j].type;
	if (s[j] == '\0')
	    break;

	if (!(i + j + 1 < nb)) // no more objects
	    break;

	if (p->proba_start < o->objs[i+j+1].proba) {
	    p->proba_end = o->objs[i+j+1].proba;
	    break;
	}
    }
    p->s = s;
}

//-----------------------------------------------------

static struct pattern *
tro_extract_pattern(const struct tr_object * o, int i, int nb,
		    double proba)
{
    struct pattern * p = malloc(sizeof(*p));
    p->proba_start = proba;
    p->proba_end   = PROBA_END;
    tro_pattern_set_str(o, i, nb, p);
    p->len = strlen(p->s);
    p->proba_total = p->proba_end - p->proba_start;
    return p;
}

//-----------------------------------------------------

static struct pattern * pattern_sub(const struct pattern * src, int len)
{
    struct pattern * p = malloc(sizeof(*p));
    p->proba_start = src->proba_start;
    p->proba_end   = src->proba_end;
    p->proba_total = (src->proba_end - src->proba_start) / src->len;
    p->s   = strndup(src->s, len);
    p->len = len;
    return p;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_pattern_influence(const struct tr_object * o1,
				    const struct tr_object * o2)
{
    int diff = o2->offset - o1->offset;
    return lf_eval(PATTERN_INFLU_LF, diff);
}

//-----------------------------------------------------

static void cnt_add_tro_patterns(struct counter * c,
				 const struct tr_object * o,
				 double influ)
{
    for (int k = 0; k < table_len(o->patterns); k++) {
	const struct pattern *p = table_get(o->patterns, k);
	if (p->s[0] == '\0')
	    continue;
	cnt_add(c, p, p->s, influ * p->proba_total);
    }
}

static struct counter * tro_pattern_new_counter(const struct tr_object * o, int i)
{
    struct counter * c = cnt_new();
    for (int j = i; j >= 0; j--) {
	double influ = tro_pattern_influence(&o->objs[j], o);
	if (influ == 0)
	    break; // j-- influence will remain 0
	cnt_add_tro_patterns(c, &o->objs[j], influ);
    }
    cnt_add_tro_patterns(c, o, 0);
    return c;
}

//-----------------------------------------------------

static inherit_fun pattern_nb = (inherit_fun) pattern_eq;
static inherit_fun pattern_total = (inherit_fun) pattern_are_same_length;

static double tro_pattern_freq(const struct tr_object * o,
			       const struct counter * c)
{
    double nb = 0;
    for (int k = 0; k < table_len(o->patterns); k++) {
	const struct pattern * p = table_get(o->patterns, k);
	double total = cnt_get_nb_inherit(c, p->s, pattern_total);
        if (total == 0)
            continue;
	double value = cnt_get_nb_inherit(c, p->s, pattern_nb);
	double p_freq = value / total;
	//p_freq = min(1, p_freq * p->len);
        //fprintf(stderr, "%s:\t%.3g / %.3g\t%g\n", p->s, value, total, p_freq);
	nb += p_freq * p->proba_total;
    }
    return nb;
}

//-----------------------------------------------------

static void pattern_free(struct pattern *p)
{
    if (p == NULL)
	return;
    free(p->s);
    free(p);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void tro_set_pattern_proba(struct tr_object * o, int i UNUSED)
{
    o->proba = lf_eval(SINGLETAP_LF, o->rest);
/*
    for (int j = i-1; j >= 0; j--) {
	if (tro_are_same_type(o, &o->objs[j])) {
	    int diff = o->offset - o->objs[j].offset;
	    o->proba = lf_eval(SINGLETAP_LF, diff);
	    return;
	}
    }
    o->proba = 1;
*/
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

static void tro_add_pattern(struct tr_object * o, struct pattern * p)
{
    for (int i = 1; i < p->len; i++) {
        struct pattern * sub_p = pattern_sub(p, i);
        table_add(o->patterns, sub_p);
    }
    p->proba_total /= p->len;
    table_add(o->patterns, p);
}

void tro_set_patterns(struct tr_object * o, int i, int nb)
{
    // taking more space for some special case
    o->patterns = table_new(pow(MAX_PATTERN_LENGTH + 1, 2));
    double proba = PROBA_START;
    while (proba < PROBA_END) {
	struct pattern * p = tro_extract_pattern(o, i, nb, proba);
	if (table_is_full(o->patterns)) {
	    fprintf(stderr, "Out of bounds %d/%d (offset %d)\n",
		    table_len(o->patterns), table_max(o->patterns),
		    o->offset);
	    print_pattern(p);
	    tro_print_pattern(o);
	    fprintf(stderr, "-----------------\n");
	}
        tro_add_pattern(o, p);
	proba = p->proba_end;
    }
}

//-----------------------------------------------------

void tro_set_pattern_freq(struct tr_object * o, int i)
{
    struct counter * c = tro_pattern_new_counter(o, i);

    double freq = tro_pattern_freq(o, c);
    o->pattern_freq = lf_eval(PATTERN_FREQ_LF, freq);
/*
    tro_print_pattern(o);
    fprintf(stderr, "Pattern value for obj n°%d: %g\n", i, o->pattern_freq);
    fprintf(stderr, "----------------------------------------------------\n");
*/
    cnt_free(c);
}

//-----------------------------------------------------

void tro_set_pattern_star(struct tr_object * o)
{
    o->pattern_star = lf_eval
	(PATTERN_SCALE_LF,
	 PATTERN_STAR_COEFF_PATTERN * o->pattern_freq);
}

//-----------------------------------------------------

void tro_free_patterns(struct tr_object * o)
{
    for(int i = 0; i < table_len(o->patterns); i++) {
	const struct pattern * p = table_get(o->patterns, i);
	// disregard const but it isn't needed anymore...
	pattern_free((struct pattern *) p);
    }
    table_free(o->patterns);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_set_pattern_proba(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	tro_set_pattern_proba(&map->object[i], i);
}

//-----------------------------------------------------

static void trm_set_type(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	tro_set_type(&map->object[i]);
}

//-----------------------------------------------------

static void trm_set_patterns(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	tro_set_patterns(&map->object[i], i, map->nb_object);
}

//-----------------------------------------------------

static void trm_set_pattern_freq(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	tro_set_pattern_freq(&map->object[i], i);
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

    /*
      Computation is based on the pattern frequency.
     */
    trm_set_pattern_proba(map);
    trm_set_type(map);
    trm_set_patterns(map);
    trm_set_pattern_freq(map);
    trm_free_patterns(map);

    trm_set_pattern_star(map);
}
