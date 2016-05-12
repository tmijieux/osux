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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "print.h"
#include "interpolation.h"

#include "stats.h"
#include "util/sum.h"

#define DEF_WEIGHT_X1 1.
#define DEF_WEIGHT_Y1 1.
#define DEF_WEIGHT_X2 1000.
#define DEF_WEIGHT_Y2 0.0001

//-----------------------------------------------------

#define TRO_COMPARE(FIELD)				\
    static int tro_compare_##FIELD(const void * o1,	\
				   const void * o2)	\
    {							\
	const struct tr_object * obj1 = o1;		\
	const struct tr_object * obj2 = o2;		\
	double f = (obj1->FIELD - obj2->FIELD);		\
	return f < 0? -1:(f > 0? 1:0);			\
    }

#define TRM_SORT(FIELD)						\
    void tro_sort_##FIELD (struct tr_object * o, int nb)	\
    {								\
	qsort(o, nb, sizeof(struct tr_object),			\
	      tro_compare_##FIELD);				\
    }								\
    void trm_sort_##FIELD (struct tr_map * map)			\
    {								\
	qsort(map->object, map->nb_object,			\
	      sizeof(struct tr_object),				\
	      tro_compare_##FIELD);				\
    }

#define TRM_QUARTILE(FIELD, NUM, DEN)  /* set min DEN !*/	\
    static double						\
    trm_q_##NUM##_##DEN##_##FIELD (struct tr_map * map)		\
    {								\
	int i = (int) (map->nb_object * ((float) NUM / DEN));	\
	if ((map->nb_object % DEN) == 0)			\
	    return ((map->object[i-1].FIELD +			\
		     map->object[i].FIELD) / 2.);		\
	return map->object[i].FIELD;				\
    }

#define TRM_MEAN(FIELD)					\
    double trm_mean_##FIELD (struct tr_map * map)	\
    {							\
	void * sum = sum_new(map->nb_object, PERF);	\
	for (int i = 0; i < map->nb_object; i++) {	\
	    sum_add(sum, map->object[i].FIELD);		\
	}						\
	return sum_compute(sum) / map->nb_object;	\
    }

// /!\ does not support threading yet
#define TRM_STATS(FIELD)					\
    struct stats * trm_stats_##FIELD (struct tr_map * map)	\
    {								\
	trm_sort_##FIELD (map);					\
	struct stats * stats = malloc(sizeof(struct stats));	\
	stats->min    = map->object[0].FIELD;			\
	stats->max    = map->object[map->nb_object - 1].FIELD;	\
	stats->spread = stats->max - stats->min;		\
	stats->mean   = trm_mean_##FIELD (map);			\
	stats->d1     = trm_q_1_10_##FIELD (map);		\
	stats->q1     = trm_q_1_4_##FIELD (map);		\
	stats->median = trm_q_1_2_##FIELD (map);		\
	stats->q3     = trm_q_3_4_##FIELD (map);		\
	stats->d9     = trm_q_9_10_##FIELD (map);		\
	trm_sort_offset(map);					\
	return stats;						\
    }

double default_weight(int i, double val)
{
    return EXP_2_PT(i, 
		    DEF_WEIGHT_X1, DEF_WEIGHT_Y1, 
		    DEF_WEIGHT_X2, DEF_WEIGHT_Y2) * val;
}

#define TRM_WEIGHT_SUM(FIELD)					\
    double trm_weight_sum_##FIELD (struct tr_map * map,		\
				   double(*weight)(int,double))	\
    {								\
	if(weight == NULL)					\
	    weight = default_weight;				\
	struct tr_object * copy = tro_copy(map->object,		\
					   map->nb_object);	\
	tro_sort_##FIELD (copy, map->nb_object);		\
	double sum = 0;						\
	for(int i = 0; i < map->nb_object; i++)	{		\
	    double d = weight(map->nb_object-i, copy[i].FIELD);	\
	    sum += d;						\
	}							\
	free(copy);						\
	return sum;						\
    }

// ---- Macro macro

#define TRM_SORT_FUNCTIONS(FIELD)		\
    TRO_COMPARE(FIELD)				\
    TRM_SORT(FIELD)

#define TRM_STATS_FUNCTIONS(FIELD)		\
    TRM_SORT_FUNCTIONS(FIELD)			\
    TRM_QUARTILE(FIELD, 1, 10) /* D1 */		\
    TRM_QUARTILE(FIELD, 1, 4)  /* Q1 */		\
    TRM_QUARTILE(FIELD, 1, 2)  /* median */	\
    TRM_QUARTILE(FIELD, 3, 4)  /* Q3 */		\
    TRM_QUARTILE(FIELD, 9, 10) /* D9 */		\
    TRM_MEAN(FIELD)				\
    TRM_STATS(FIELD)				\
    TRM_WEIGHT_SUM(FIELD)

//-----------------------------------------------------

TRM_SORT_FUNCTIONS(offset)

TRM_STATS_FUNCTIONS(density_star)
TRM_STATS_FUNCTIONS(reading_star)
TRM_STATS_FUNCTIONS(pattern_star)
TRM_STATS_FUNCTIONS(accuracy_star)
TRM_STATS_FUNCTIONS(final_star)

//-----------------------------------------------------

double stats_stars(struct stats * stats, struct stats * coeff)
{
    double stars = ((coeff->median * stats->median +
		     coeff->mean   * stats->mean +
		     coeff->d1     * stats->d1 +
		     coeff->d9     * stats->d9 +
		     coeff->q1     * stats->q1 +
		     coeff->q3     * stats->q3) /
		    coeff->scaling);
    free(stats);
    return stars;
}

//-----------------------------------------------------

void stats_print(struct stats * stats)
{
    fprintf(OUTPUT_INFO, "-------- Stats --------\n");
    fprintf(OUTPUT_INFO, "min:    %g\n", stats->min);
    fprintf(OUTPUT_INFO, "max:    %g\n", stats->max);
    fprintf(OUTPUT_INFO, "spread: %g\n", stats->spread);

    fprintf(OUTPUT_INFO, "\n");
    fprintf(OUTPUT_INFO, "mean:   %g\n", stats->mean);
    fprintf(OUTPUT_INFO, "\n");
  
    fprintf(OUTPUT_INFO, "D1:     %g\n", stats->d1);
    fprintf(OUTPUT_INFO, "Q1:     %g\n", stats->q1);
    fprintf(OUTPUT_INFO, "median: %g\n", stats->median);
    fprintf(OUTPUT_INFO, "Q3:     %g\n", stats->q3);
    fprintf(OUTPUT_INFO, "D9:     %g\n", stats->d9);
}

