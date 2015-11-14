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

#include "util/hashtable/hashtable.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "sum.h"
#include "stats.h"

#include "pattern.h"

static void add_new_pattern(const char * s, double d1, double d2);

static void trm_compute_full_alt_stream(struct tr_map * map);

//--------------------------------------------------

#define ADD_NEW_PATTERN(S, P) add_new_pattern(S, P##_1, P##_2)
#define REMOVE_PATTERN(S)					\
  ht_get_entry(ht_pattern, S, &p);				\
  free(p);

#define MAX_PATTERN_LENGTH 4

struct pattern
{
  double d1;
  double d2;
};

static struct hash_table * ht_pattern;

//--------------------------------------------------

// First Object
// 1 pattern
#define D_1    1
#define K_1    1
// 2 pattern
#define DD_1   2
#define DK_1   2.5
#define KD_1   2.5
#define KK_1   2
// 3 pattern
#define DDD_1  2
#define DDK_1  2.2
#define DKD_1  2.5
#define DKK_1  2.8
#define KDD_1  2.2
#define KDK_1  2.5
#define KKD_1  2.2
#define KKK_1  2
// 4 pattern
#define DDDD_1 2.1
#define DDDK_1 2.6
#define DDKD_1 2.3
#define DDKK_1 2.2
#define DKDD_1 3.1
#define DKDK_1 2.3
#define DKKD_1 2.5
#define DKKK_1 2.3

#define KDDD_1 2.3
#define KDDK_1 2.5
#define KDKD_1 2.3
#define KDKK_1 3.1
#define KKDD_1 2.2
#define KKDK_1 2.3
#define KKKD_1 2.6
#define KKKK_1 2.1

// Second Object
// 1 pattern
#define D_2    0
#define K_2    0
// 2 pattern
#define DD_2   2
#define DK_2   2.5
#define KD_2   2.5
#define KK_2   2
// 3 pattern
#define DDD_2  2
#define DDK_2  2.2
#define DKD_2  2.5
#define DKK_2  2.8
#define KDD_2  2.2
#define KDK_2  2.5
#define KKD_2  2.2
#define KKK_2  2
// 4 pattern
#define DDDD_2 2.1
#define DDDK_2 2.6
#define DDKD_2 2.3
#define DDKK_2 2.2
#define DKDD_2 3.1
#define DKDK_2 2.3
#define DKKD_2 2.5
#define DKKK_2 2.3

#define KDDD_2 2.3
#define KDDK_2 2.5
#define KDKD_2 2.3
#define KDKK_2 3.1
#define KKDD_2 2.2
#define KKDK_2 2.3
#define KKKD_2 2.6
#define KKKK_2 2.1

//--------------------------------------------------

// coeff for stats
#define PATTERN_COEFF_MEDIAN 0.7
#define PATTERN_COEFF_MEAN   0.
#define PATTERN_COEFF_D1     0.
#define PATTERN_COEFF_D9     0.
#define PATTERN_COEFF_Q1     0.3
#define PATTERN_COEFF_Q3     0.

// scaling
#define PATTERN_STAR_SCALING 100.

// stats module
TRM_STATS_HEADER(pattern_star, PATTERN)

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void add_new_pattern(const char * s, double d1, double d2)
{
  struct pattern * p = malloc(sizeof(*p));
  p->d1 = d1;
  p->d2 = d2;
  ht_add_entry(ht_pattern, s, p);
}

__attribute__((constructor))
static void ht_pattern_init()
{
  ht_pattern = ht_create(NULL);
  // 1 pattern list
  ADD_NEW_PATTERN("d", D);
  ADD_NEW_PATTERN("k", K);
  // 2 pattern list
  ADD_NEW_PATTERN("dd", DD);
  ADD_NEW_PATTERN("dk", DK);
  ADD_NEW_PATTERN("kd", KD);
  ADD_NEW_PATTERN("kk", KK);
  // 3 pattern list
  ADD_NEW_PATTERN("ddd", DDD);
  ADD_NEW_PATTERN("ddk", DDK);
  ADD_NEW_PATTERN("dkd", DKD);
  ADD_NEW_PATTERN("dkk", DKK);
  ADD_NEW_PATTERN("kdd", KDD);
  ADD_NEW_PATTERN("kdk", KDK);
  ADD_NEW_PATTERN("kkd", KKD);
  ADD_NEW_PATTERN("kkk", KKK);
  // 4 pattern list
  ADD_NEW_PATTERN("dddd", DDDD);
  ADD_NEW_PATTERN("dddk", DDDK);
  ADD_NEW_PATTERN("ddkd", DDKD);
  ADD_NEW_PATTERN("ddkk", DDKK);
  ADD_NEW_PATTERN("dkdd", DKDD);
  ADD_NEW_PATTERN("dkdk", DKDK);
  ADD_NEW_PATTERN("dkkd", DKKD);
  ADD_NEW_PATTERN("dkkk", DKKK);
  ADD_NEW_PATTERN("kddd", KDDD);
  ADD_NEW_PATTERN("kddk", KDDK);
  ADD_NEW_PATTERN("kdkd", KDKD);
  ADD_NEW_PATTERN("kdkk", KDKK);
  ADD_NEW_PATTERN("kkdd", KKDD);
  ADD_NEW_PATTERN("kkdk", KKDK);
  ADD_NEW_PATTERN("kkkd", KKKD);
  ADD_NEW_PATTERN("kkkk", KKKK);
}
  
//--------------------------------------------------

__attribute__((destructor))
static void ht_pattern_free()
{
  struct pattern * p = NULL;
  // 1 pattern
  REMOVE_PATTERN("d");
  REMOVE_PATTERN("k");
  // 2 pattern
  REMOVE_PATTERN("dd");
  REMOVE_PATTERN("dk");
  REMOVE_PATTERN("kd");
  REMOVE_PATTERN("kk");
  // 3 pattern
  REMOVE_PATTERN("ddd");
  REMOVE_PATTERN("ddk");
  REMOVE_PATTERN("dkd");
  REMOVE_PATTERN("dkk");
  REMOVE_PATTERN("kdd");
  REMOVE_PATTERN("kdk");
  REMOVE_PATTERN("kkd");
  REMOVE_PATTERN("kkk");
  // 3 pattern
  REMOVE_PATTERN("dddd");
  REMOVE_PATTERN("dddk");
  REMOVE_PATTERN("ddkd");
  REMOVE_PATTERN("ddkk");
  REMOVE_PATTERN("dkdd");
  REMOVE_PATTERN("dkdk");
  REMOVE_PATTERN("dkkd");
  REMOVE_PATTERN("dkkk");  
  REMOVE_PATTERN("kddd");
  REMOVE_PATTERN("kddk");
  REMOVE_PATTERN("kdkd");
  REMOVE_PATTERN("kdkk");
  REMOVE_PATTERN("kkdd");
  REMOVE_PATTERN("kkdk");
  REMOVE_PATTERN("kkkd");
  REMOVE_PATTERN("kkkk");
	    
  ht_free(ht_pattern);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_full_alt_stream(struct tr_map * map)
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
      map->object[i].pattern_full_alt1 = p->d1;
      map->object[i+1].pattern_full_alt2 = p->d2;
      if(strlen(s) == 1)
	i++;
      else
	i+=2;
    }
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_pattern (struct tr_map * map)
{
  trm_compute_full_alt_stream(map);
}
