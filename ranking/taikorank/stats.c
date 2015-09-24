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

#include <stdlib.h>
#include <math.h>

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"

#include "stats.h"
#include "sum/sum.h"

//-----------------------------------------------------

#define TRO_COMPARE(FIELD)					\
  int tro_compare_##FIELD (const struct tr_object * obj1,	\
			   const struct tr_object * obj2)	\
 {								\
    return obj1->FIELD - obj2->FIELD;				\
 }

#define TRM_SORT(FIELD)						\
  void trm_sort_##FIELD (struct tr_map * map)			\
  {								\
    qsort(map->object, map->nb_object,				\
	  sizeof(struct tr_object),				\
	  (int (*)(const void *, const void *))			\
	  tro_compare_##FIELD);					\
  }

#define TRM_QUARTILE(FIELD, NUM, DEN)  /* set min DEN !*/	\
  double trm_q_##NUM##_##DEN##_##FIELD (struct tr_map * map)	\
  {								\
    int i = (int) (map->nb_object * ((float) NUM / DEN));	\
    if ((map->nb_object % DEN) == 0)				\
      return							\
	(map->object[i-1].FIELD + map->object[i].FIELD) / 2.;	\
    return map->object[i].FIELD;				\
  }

#define TRM_MEAN(FIELD)					\
  double trm_mean_##FIELD (struct tr_map * map)		\
  {							\
    void * sum = sum_new(map->nb_object, PERF);		\
    for (int i = 0; i < map->nb_object; i++)		\
      {							\
	sum_add(sum, map->object[i].FIELD);		\
      }							\
    return sum_compute(sum);				\
  }

#define TRM_STATS(FIELD)					\
  struct stats * trm_stats_##FIELD (struct tr_map * map)	\
  {								\
    trm_sort_##FIELD (map);					\
    struct stats * stats = malloc(sizeof(struct stats));	\
    stats->mean   = trm_mean_##FIELD (map);			\
    stats->q1     = trm_q_1_4_##FIELD (map);			\
    stats->median = trm_q_1_2_##FIELD (map);			\
    stats->q3     = trm_q_3_4_##FIELD (map);			\
    trm_sort_offset(map);					\
    return stats;						\
  }

#define TRM_SORT_FUNCTIONS(FIELD)		\
  TRO_COMPARE(FIELD)				\
  TRM_SORT(FIELD)

#define TRM_STATS_FUNCTIONS(FIELD)		\
  TRM_SORT_FUNCTIONS(FIELD)			\
  TRM_QUARTILE(FIELD, 1, 4) /* Q1 */		\
  TRM_QUARTILE(FIELD, 1, 2) /* median */	\
  TRM_QUARTILE(FIELD, 3, 4) /* Q3 */		\
  TRM_MEAN(FIELD)				\
  TRM_STATS(FIELD)

//-----------------------------------------------------

TRM_SORT_FUNCTIONS(offset)

TRM_STATS_FUNCTIONS(density_star)
TRM_STATS_FUNCTIONS(reading_star)
TRM_STATS_FUNCTIONS(pattern_star)
TRM_STATS_FUNCTIONS(accuracy_star)

//-----------------------------------------------------
