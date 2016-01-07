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
#include <stdio.h>

#include "taiko_ranking_object.h"
#include "taiko_ranking_map.h"
#include "taiko_ranking_score.h"

#include "config.h"
#include "print.h"
#include "tr_db.h"

static void trs_acc(struct tr_score * score);
static void trs_print_and_db(struct tr_score * score);

//--------------------------------------------------

void trs_main(const struct tr_map * map, int mods)
{
  struct tr_score * score = trs_new(map, mods, SCORE_ACC);
  trs_compute(score);
  trs_free(score);
}

//--------------------------------------------------

struct tr_score * trs_new(const struct tr_map * map, int mods,
			  double acc)
{
  struct tr_score * score = malloc(sizeof(*score));
  score->origin = map;
  
  score->map = trm_copy(map);
  trm_set_mods(score->map, mods);

  score->acc = acc;

  return score;
}

//--------------------------------------------------

void trs_free(struct tr_score * score)
{
  trm_free(score->map);
  free(score);
}

//--------------------------------------------------

static void trs_print_and_db(struct tr_score * score)
{
  if(OPT_PRINT_TRO)
    trm_print_tro(score->map, FILTER_APPLY);
  if(OPT_PRINT_YAML)
    trm_print_yaml(score->map);
  else
    trs_print(score);
  
  if(OPT_DATABASE)
    trm_db_insert(score->map);
}

//--------------------------------------------------

void trs_compute(struct tr_score * score)
{
  trm_compute_stars(score->map);
  trs_print_and_db(score);

  while(score->acc < score->map->acc)
    {
      trm_pattern_free(score->map);
      int i = TRM_METHOD_GET_TRO(score->map); 
      // TODO: for bonus...
      trm_set_miss_tro(score->map, i);
      trs_acc(score);
      trm_compute_stars(score->map);

      trs_print_and_db(score);
    }
}

//--------------------------------------------------

void trs_print(struct tr_score * score)
{
  fprintf(OUTPUT_INFO, "Score: %.5g%% \t(aim: %g%%)\n",
	  score->map->acc * COEFF_MAX_ACC, 
	  score->acc * COEFF_MAX_ACC);  
  trm_print(score->map);
}

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

static void trs_acc(struct tr_score * score)
{
  score->map->acc = ((score->map->great + 
		      score->map->good * 0.5) /
		     score->map->max_combo);
}

