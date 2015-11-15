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
#include <string.h>
#include <stdarg.h>

#include "util/hashtable/hashtable.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "sum.h"
#include "stats.h"

#include "pattern.h"

//--------------------------------------------------

struct pattern
{
  double * d;
};

static struct hash_table * ht_pattern;

//--------------------------------------------------

static void trm_set_pattern_0(struct tr_map * map);
// ^ to move ?

static void add_new_pattern(const char * s, ...);
static void remove_pattern(const char * s, struct pattern ** p);

static void trm_compute_pattern_full_alt_stream(struct tr_map * map);

static void trm_compute_pattern_star(struct tr_map * map);

//--------------------------------------------------

#define MAX_PATTERN_LENGTH  4
#define LENGTH_PATTERN_USED 2

// 1 pattern
#define D    1,   0
#define K    1,   0
// 2 pattern
#define DD   2,   2
#define DK   2.5, 2.5
#define KD   2.5, 2.5
#define KK   2,   2
// 3 pattern
#define DDD  2,   2
#define DDK  2.2, 2.2
#define DKD  2.5, 2.5
#define DKK  2.8, 2.8
#define KDD  2.2, 2.2
#define KDK  2.5, 2.5
#define KKD  2.2, 2.2
#define KKK  2,   2
// 4 pattern
#define DDDD 2.1, 2.1
#define DDDK 2.6, 2.6
#define DDKD 2.3, 2.3
#define DDKK 2.2, 2.2
#define DKDD 3.1, 3.1
#define DKDK 2.3, 2.3
#define DKKD 2.5, 2.5
#define DKKK 2.3, 2.3

#define KDDD 2.3, 2.3
#define KDDK 2.5, 2.5
#define KDKD 2.3, 2.3
#define KDKK 3.1, 3.1
#define KKDD 2.2, 2.2
#define KKDK 2.3, 2.3
#define KKKD 2.6, 2.6
#define KKKK 2.1, 2.1

//--------------------------------------------------

// coeff for star
#define PATTERN_STAR_COEFF_ALT 1.

// coeff for stats
#define PATTERN_COEFF_MEDIAN 0.1
#define PATTERN_COEFF_MEAN   0.2
#define PATTERN_COEFF_D1     0.3
#define PATTERN_COEFF_D9     0.3
#define PATTERN_COEFF_Q1     0.
#define PATTERN_COEFF_Q3     0.

// scaling
#define PATTERN_STAR_SCALING 1.

// stats module
TRM_STATS_HEADER(pattern_star, PATTERN)

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void add_new_pattern(const char * s, ...)
{
  struct pattern * p = malloc(sizeof(*p));
  p->d = malloc(sizeof(double) * LENGTH_PATTERN_USED);
  
  va_list vl;
  va_start(vl, s);
  for (int i = 0; i < LENGTH_PATTERN_USED; ++i)
    p->d[i] = va_arg(vl, double);
  va_end(vl);
  
  ht_add_entry(ht_pattern, s, p);
}

static void remove_pattern(const char * s, struct pattern ** p)
{
  ht_get_entry(ht_pattern, s, p);
  free((*p)->d);
  free(*p);
}

//-----------------------------------------------------

__attribute__((constructor))
static void ht_pattern_init()
{
  ht_pattern = ht_create(NULL);
  // 1 pattern list
  add_new_pattern("d", D);
  add_new_pattern("k", K);
  // 2 pattern list
  add_new_pattern("dd", DD);
  add_new_pattern("dk", DK);
  add_new_pattern("kd", KD);
  add_new_pattern("kk", KK);
  // 3 pattern list
  add_new_pattern("ddd", DDD);
  add_new_pattern("ddk", DDK);
  add_new_pattern("dkd", DKD);
  add_new_pattern("dkk", DKK);
  add_new_pattern("kdd", KDD);
  add_new_pattern("kdk", KDK);
  add_new_pattern("kkd", KKD);
  add_new_pattern("kkk", KKK);
  // 4 pattern list
  add_new_pattern("dddd", DDDD);
  add_new_pattern("dddk", DDDK);
  add_new_pattern("ddkd", DDKD);
  add_new_pattern("ddkk", DDKK);
  add_new_pattern("dkdd", DKDD);
  add_new_pattern("dkdk", DKDK);
  add_new_pattern("dkkd", DKKD);
  add_new_pattern("dkkk", DKKK);
  
  add_new_pattern("kddd", KDDD);
  add_new_pattern("kddk", KDDK);
  add_new_pattern("kdkd", KDKD);
  add_new_pattern("kdkk", KDKK);
  add_new_pattern("kkdd", KKDD);
  add_new_pattern("kkdk", KKDK);
  add_new_pattern("kkkd", KKKD);
  add_new_pattern("kkkk", KKKK);
}
  
//--------------------------------------------------

__attribute__((destructor))
static void ht_pattern_free()
{
  struct pattern * p = NULL;
  // 1 pattern
  remove_pattern("d", &p);
  remove_pattern("k", &p);
  // 2 pattern
  remove_pattern("dd", &p);
  remove_pattern("dk", &p);
  remove_pattern("kd", &p);
  remove_pattern("kk", &p);
  // 3 pattern
  remove_pattern("ddd", &p);
  remove_pattern("ddk", &p);
  remove_pattern("dkd", &p);
  remove_pattern("dkk", &p);
  remove_pattern("kdd", &p);
  remove_pattern("kdk", &p);
  remove_pattern("kkd", &p);
  remove_pattern("kkk", &p);
  // 3 pattern
  remove_pattern("dddd", &p);
  remove_pattern("dddk", &p);
  remove_pattern("ddkd", &p);
  remove_pattern("ddkk", &p);
  remove_pattern("dkdd", &p);
  remove_pattern("dkdk", &p);
  remove_pattern("dkkd", &p);
  remove_pattern("dkkk", &p);  
  remove_pattern("kddd", &p);
  remove_pattern("kddk", &p);
  remove_pattern("kdkd", &p);
  remove_pattern("kdkk", &p);
  remove_pattern("kkdd", &p);
  remove_pattern("kkdk", &p);
  remove_pattern("kkkd", &p);
  remove_pattern("kkkk", &p);
	    
  ht_free(ht_pattern);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_pattern_full_alt_stream(struct tr_map * map)
{ 
  int i = 0;
  while(i < map->nb_object)
    {
      char s[MAX_PATTERN_LENGTH + 1];
      struct pattern * p = NULL;
      
      for (int j = 0; j < MAX_PATTERN_LENGTH; j++)
	{
	  if (tro_is_bonus(&map->object[i+j]))
	    {
	      s[j] = 0;
	      break;
	    }
	  else if (tro_is_don(&map->object[i+j]))
	    {
	      s[j] = 'd';
	    }
	  else // if (tro_is_kat(&map->object[i+j]))
	    {
	      s[j] = 'k';
	    }
	  
	  if (tro_is_big(&map->object[i+j]))
	    {
	      s[j+1] = 0;
	      break;
	    }
	}
      s[4] = 0;
            
      int ret = ht_get_entry(ht_pattern, s, &p);
      if (ret != 0) // when s[0] = 0 (bonus) or not found (error)
	{
	  /*if (s[0] != 0)
	    printf("Pattern %s introuvable...\n", s);*/
	  i++;
	  continue;
	}
      
      //printf("%s: %g %g\n", s, p->d1, p->d2);
      
      map->object[i].pattern_alt1 = p->d[0];
      map->object[i+1].pattern_alt1 = p->d[1];
      
      if(strlen(s) == 1)
	i++;
      else
	i+=2;
    }
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_pattern_star(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].pattern_star =
	(PATTERN_STAR_COEFF_ALT * map->object[i].pattern_alt1 +
	 PATTERN_STAR_COEFF_ALT * map->object[i].pattern_alt2);
    }

  map->pattern_star = trm_stats_compute_pattern_star(map); 
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_set_pattern_0(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].pattern_alt1 = 0;
      map->object[i].pattern_alt2 = 0;
    }
}

void trm_compute_pattern (struct tr_map * map)
{
  trm_set_pattern_0(map);
  trm_compute_pattern_full_alt_stream(map);
  trm_compute_pattern_star(map);
}
