/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
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
#include "interpolation.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "util/hashtable/hash_table.h"
#include "util/list/list.h"
#include "yaml/yaml2.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "sum.h"
#include "stats.h"
#include "cst_yaml.h"
#include "print.h"

#include "pattern.h"

struct pattern
{
  double * d;
};

static int pattern_set;
static struct hash_table * ht_pattern;
static struct hash_table * ht_cst;

//--------------------------------------------------

static void remove_pattern(const char * s, void * p, void * null);
static void ht_pattern_init();

static double tro_singletap_proba(struct tr_object * obj);
static struct pattern * trm_get_pattern(struct tr_map * map,
					int i, double proba_alt);

static void trm_pattern_alloc(struct tr_map * map);
static void trm_compute_pattern_proba(struct tr_map * map);
static void trm_compute_pattern_full_alt(struct tr_map * map);
static void trm_compute_pattern_singletap(struct tr_map * map);

static void trm_compute_pattern_star(struct tr_map * map);

//--------------------------------------------------

#define PATTERN_FILE  "pattern_cst.yaml"
#define PATTERN_STATS "pattern_stats"

// coeff for singletap proba
#define SINGLETAP_MIN cst_f(ht_cst, "singletap_min")
#define SINGLETAP_MAX cst_f(ht_cst, "singletap_max")
#define TIME_MIN      cst_f(ht_cst, "time_min")
#define TIME_MAX      cst_f(ht_cst, "time_max")
// 160ms ~ 180bpm 1/2
// 125ms = 240bpm 1/2

#define PROBA_START cst_f(ht_cst, "proba_start")  
#define PROBA_END   cst_f(ht_cst, "proba_end")
#define PROBA_STEP  cst_f(ht_cst, "proba_step")

// coeff for star
#define PATTERN_STAR_COEFF_ALT cst_f(ht_cst, "star_alt")
#define PATTERN_STAR_COEFF_SIN cst_f(ht_cst, "star_sin")

// pattern
#define MAX_PATTERN_LENGTH  cst_f(ht_cst, "max_pattern_length")
#define LENGTH_PATTERN_USED cst_f(ht_cst, "length_pattern_used")

// coeff for stats
#define PATTERN_COEFF_MEDIAN 0.5
#define PATTERN_COEFF_MEAN   0.3
#define PATTERN_COEFF_D1     0.
#define PATTERN_COEFF_D9     0.2
#define PATTERN_COEFF_Q1     0.
#define PATTERN_COEFF_Q3     0.

// scaling
#define PATTERN_STAR_SCALING 5.0

// stats module
TRM_STATS_HEADER(pattern_star, PATTERN)

#define cst_assert(COND, MSG)			\
  if(!(COND))					\
    {						\
      tr_error(MSG);				\
      pattern_set = 0;				\
      return;					\
    }

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

__attribute__((constructor))
static void ht_cst_init_pattern()
{
  ht_cst = cst_get_ht(PATTERN_FILE);
  if(ht_cst == NULL)
    pattern_set = 0;
  else
    {
      pattern_set = 1;
      ht_pattern_init();
    }
}

//-----------------------------------------------------

static void ht_pattern_init()
{  
  ht_pattern = ht_create(0, NULL);

  struct yaml_wrap * yw_l = NULL;
  ht_get_entry(ht_cst, "patterns", &yw_l);
  cst_assert(yw_l != NULL, "Pattern list not found.");
  cst_assert(yw_l->type == YAML_SEQUENCE,
	     "Pattern list is not a list.");
  
  struct list * pattern_l = yw_l->content.sequence;
  for(int i = 1; i <= list_size(pattern_l); i++)
    {
      struct yaml_wrap * yw_subl = list_get(pattern_l, i);
      cst_assert(yw_subl->type == YAML_SEQUENCE,
		  "Pattern structure is not a list.");
      struct list * pattern_data = yw_subl->content.sequence;

      cst_assert(LENGTH_PATTERN_USED + 1 == list_size(pattern_data),
		  "Pattern structure does not have the right size.");
      
      struct yaml_wrap * yw_name = list_get(pattern_data, 1);
      cst_assert(yw_name->type == YAML_SCALAR,
		  "Pattern name is not a scalar.");
      char * name = yw_name->content.scalar;

      struct pattern * p = calloc(sizeof(*p), 1);
      p->d = calloc(sizeof(double), LENGTH_PATTERN_USED);
      
      for(int j = 2; j <= list_size(pattern_data); j++)
	{
	  struct yaml_wrap * yw_s = list_get(pattern_data, j);
	  cst_assert(yw_s->type == YAML_SCALAR,
		      "Pattern value is not a scalar.");
	  p->d[j-2] = atof(yw_s->content.scalar);
	}
      ht_add_entry(ht_pattern, name, p);
    }
}
  
//-----------------------------------------------------

static void remove_pattern(const char * s, void * p, void * null)
{
  free(((struct pattern *) p)->d);
  free(p);
}

//--------------------------------------------------

__attribute__((destructor))
static void ht_pattern_free()
{
  ht_for_each(ht_pattern, remove_pattern, NULL);
  ht_free(ht_pattern);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_singletap_proba(struct tr_object * obj)
{
  double time = obj->rest;
  if (time > TIME_MAX)
    time = TIME_MAX;

  return POLY_2_PT(time,
		   TIME_MIN, SINGLETAP_MIN,
		   TIME_MAX, SINGLETAP_MAX);
}

//-----------------------------------------------------

static struct pattern * trm_get_pattern(struct tr_map * map,
					int i, double proba_alt)
{
  char * s = calloc(sizeof(char), MAX_PATTERN_LENGTH + 1);
  for (int j = 0; (j < MAX_PATTERN_LENGTH &&
		   i + j < map->nb_object); j++)
    {
      if (tro_is_bonus(&map->object[i+j]))
	{
	  s[j] = 0;
	  break; // continue ?
	}
      else if (tro_is_don(&map->object[i+j]))
	s[j] = 'd';
      else // if (tro_is_kat(&map->object[i+j]))
	s[j] = 'k';
      
      if (tro_is_big(&map->object[i+j]) ||
	  map->object[i+j].proba > proba_alt)
	{
	  s[j+1] = 0;
	  break;
	}
    }

  struct pattern * p = NULL;
  int ret = ht_get_entry(ht_pattern, s, &p);
  if (ret != 0) // when s[0] = 0 (bonus) or not found (error)
    {
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
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].proba = tro_singletap_proba(&map->object[i]);
    }  
}

//-----------------------------------------------------

static void trm_compute_pattern_full_alt(struct tr_map * map)
{ 
  for(int i = 0; i < map->nb_object; i++)
    {
      struct pattern * p = trm_get_pattern(map, i, 1);
      if (p == NULL)
	continue;

      for (int j = 0; (j < LENGTH_PATTERN_USED &&
		       i + j < map->nb_object); j++)
	map->object[i+j].alt[j] = p->d[j];
    }
}

//-----------------------------------------------------

static void trm_compute_pattern_singletap(struct tr_map * map)
{
  float proba;
  for(proba = PROBA_START; proba <= PROBA_END; proba += PROBA_STEP)
    for(int i = 0; i < map->nb_object; i++)
      {
	struct pattern * p = trm_get_pattern(map, i, proba);
	if (p == NULL)
	  continue;
	
	for (int j = 0; (j < LENGTH_PATTERN_USED &&
			 i + j < map->nb_object); j++)
	  map->object[i+j].singletap[j] += proba * p->d[j];
      }
}


//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_pattern_star(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].pattern_star = 0;
      for (int j = 0; j < LENGTH_PATTERN_USED; j++)
	{
	  map->object[i].pattern_star +=
	    PATTERN_STAR_COEFF_ALT * map->object[i].alt[j] +
	    PATTERN_STAR_COEFF_SIN * map->object[i].singletap[j];
	}
    }
  struct stats * stats = cst_stats(ht_cst, PATTERN_STATS);
  map->pattern_star = trm_stats_2_compute_pattern_star(map, stats); 
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_pattern_alloc(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].alt =
	calloc(sizeof(double), LENGTH_PATTERN_USED);
      map->object[i].singletap =
	calloc(sizeof(double), LENGTH_PATTERN_USED);
    }  
}

//-----------------------------------------------------

void trm_compute_pattern (struct tr_map * map)
{
  if(!pattern_set)
    {
      tr_error("Unable to compute pattern stars.");
      return;
    }
  
  trm_pattern_alloc(map);
  trm_compute_pattern_proba(map);
  trm_compute_pattern_full_alt(map);
  trm_compute_pattern_singletap(map);
  
  trm_compute_pattern_star(map);
}
