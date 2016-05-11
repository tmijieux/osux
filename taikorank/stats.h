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

#ifndef STATS_H
#define STATS_H

//-----------------------------------------------------

#define TRM_SORT_HEADER(FIELD)					\
    void tro_sort_##FIELD (struct tr_object * o, int nb);	\
    void trm_sort_##FIELD (struct tr_map * map);

#define TRM_STATS_HEADER(FIELD)					\
    TRM_SORT_HEADER(FIELD)					\
    double trm_mean_##FIELD (struct tr_map * map);		\
    struct stats * trm_stats_##FIELD (struct tr_map * map);	\
    double trm_weight_sum_##FIELD(struct tr_map * map,		\
				  double(*weight)(int,double));

//-----------------------------------------------------

struct stats
{
    double min;
    double max;
    double spread;
  
    double mean;
    double d1;
    double q1;
    double median;
    double q3;
    double d9;

    double scaling;
};

//-----------------------------------------------------

TRM_SORT_HEADER(offset)

TRM_STATS_HEADER(pattern_star)
TRM_STATS_HEADER(density_star)
TRM_STATS_HEADER(reading_star)
TRM_STATS_HEADER(accuracy_star)
TRM_STATS_HEADER(final_star)

double stats_stars(struct stats * stats, struct stats * coeff);
void stats_print(struct stats * stats);

#endif //STATS_H
