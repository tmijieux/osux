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

#ifndef STATS_H
#define STATS_H

//-----------------------------------------------------

#define TRM_STATS_COMPUTE_STARS(FIELD, MAJ)			\
  double trm_stats_compute_##FIELD (struct tr_map * map)	\
  {								\
    struct stats * s = trm_stats_##FIELD (map);			\
    /*stats_print(s);*/						\
    double stars = (( MAJ##_COEFF_MEDIAN * s->median +		\
		      MAJ##_COEFF_MEAN   * s->mean +		\
		      MAJ##_COEFF_D1     * s->d1 +		\
		      MAJ##_COEFF_D9     * s->d9 +		\
		      MAJ##_COEFF_Q1     * s->q1 +		\
		      MAJ##_COEFF_Q3     * s->q3) /		\
		    MAJ##_STAR_SCALING);			\
    free(s);							\
    return stars;						\
  }

#define TRM_STATS_COMPUTE_STARS_2(FIELD)			\
  double trm_stats_2_compute_##FIELD (struct tr_map * map,	\
				      struct stats * coeff)	\
  {								\
    struct stats * s = trm_stats_##FIELD (map);			\
    /*stats_print(s);*/						\
    double stars = (( coeff->median * s->median +		\
		      coeff->mean   * s->mean +			\
		      coeff->d1     * s->d1 +			\
		      coeff->d9     * s->d9 +			\
		      coeff->q1     * s->q1 +			\
		      coeff->q3     * s->q3) /			\
		    coeff->scaling);				\
    free(s);							\
    free(coeff);						\
    return stars;						\
  }

//-----------------------------------------------------

#define TRM_SORT_HEADER(FIELD)			\
  void trm_sort_##FIELD (struct tr_map * map);

/**
 * to use in .c files after all the coefficient definitions
 * -> DENSITY_COEFF_D1...
 * FIELD ex: density_star
 * MAJ   ex: DENSITY
 */
#define TRM_STATS_HEADER(FIELD, MAJ)				\
  TRM_SORT_HEADER(FIELD)					\
  double trm_mean_##FIELD (struct tr_map * map);		\
  struct stats * trm_stats_##FIELD (struct tr_map * map);	\
  TRM_STATS_COMPUTE_STARS(FIELD, MAJ)				\
  TRM_STATS_COMPUTE_STARS_2(FIELD)
  

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

void stats_print(struct stats * stats);

#endif //STATS_H
