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

#include "taiko_ranking_object.h"
#include "taiko_ranking_map.h"
#include "taiko_ranking_score.h"

#include "config.h"
#include "print.h"
#include "tr_db.h"
#include "tr_mods.h"

static void trs_print_and_db(struct tr_score * score);
static void trs_compute(struct tr_score * score);

static void trs_prepare_acc(struct tr_score * sc, double acc);
static void trs_prepare_ggm(struct tr_score * sc, int good, int miss);

static struct tr_score * trs_new(const struct tr_map * map);
static void trs_free(struct tr_score * score);
static void trs_print(struct tr_score * score);

//--------------------------------------------------

void trs_main(const struct tr_map * map, int mods)
{
    struct tr_score * score = trs_new(map);
    score->map = trm_copy(score->origin);
    trm_set_mods(score->map, mods);

    // modifications
    if(OPT_FLAT)
	trm_flat_big(score->map);
    if(OPT_NO_BONUS)
	trm_remove_bonus(score->map);


    if(OPT_SCORE_INPUT == SCORE_INPUT_GGM)
	trs_prepare_ggm(score, OPT_SCORE_GOOD, OPT_SCORE_MISS);
    else
	trs_prepare_acc(score, OPT_SCORE_ACC);

    trs_compute(score);
    trs_free(score);
}

//--------------------------------------------------

static void trs_prepare_ggm(struct tr_score * sc, int good, int miss)
{
    if(good < 0)
	good = 0;
    if(miss < 0)
	miss = 0;

    while(good + miss > sc->origin->great) {
	if(good > 0)
	    good--;
	else
	    miss--;
    }

    sc->great = sc->origin->great - good - miss;
    sc->good  = good;
    sc->miss  = miss;
    sc->acc   = compute_acc(sc->great, sc->good, sc->miss);
}

static void trs_prepare_acc(struct tr_score * sc, double acc)
{
    if(acc < 0)
	acc = 0;
    else if(acc > MAX_ACC * COEFF_MAX_ACC)
	acc = MAX_ACC * COEFF_MAX_ACC;

    sc->great = sc->origin->great;
    sc->good  = sc->origin->good;
    sc->miss  = sc->origin->miss;
    sc->acc   = compute_acc(sc->great, sc->good, sc->miss); 
    while(sc->acc > acc) {
	double try = compute_acc(sc->great-1, sc->good+1, sc->miss);
	if(try <= acc) {
	    sc->great--;
	    sc->good++;
	    sc->acc = try;
	} else {
	    sc->great--;
	    sc->miss++;
	    sc->acc = compute_acc(sc->great, sc->good, sc->miss);
	}
    }
}

//--------------------------------------------------

static struct tr_score * trs_new(const struct tr_map * map)
{
    struct tr_score * sc = malloc(sizeof(*sc));
    sc->origin = map;
    return sc;
}

//--------------------------------------------------

static void trs_free(struct tr_score * score)
{
    if(score == NULL)
	return;
    trm_free(score->map);
    free(score);
}

//--------------------------------------------------

static void trs_print_and_db(struct tr_score * score)
{
    if(OPT_PRINT_TRO)
	trm_print_tro(score->map, OPT_PRINT_FILTER);
    if(OPT_PRINT_YAML)
	trm_print_yaml(score->map);
    else
	trs_print(score);
  
    if(OPT_DATABASE)
	trm_db_insert(score->map);
}

//--------------------------------------------------

static int trs_is_finished(struct tr_score * score)
{
    return ((score->great == score->map->great) &&
	    (score->good  == score->map->good) &&
	    (score->miss  == score->map->miss));
}

//--------------------------------------------------

static void trs_compute(struct tr_score * score)
{
    trm_apply_mods(score->map);
    trm_compute_stars(score->map);
    trs_print_and_db(score);

    while(!trs_is_finished(score)) {
	if(OPT_SCORE_QUICK == 0)      
	    trm_pattern_free(score->map);

	int i = TRM_METHOD_GET_TRO(score->map);
	if(score->miss != score->map->miss)
	    trm_set_tro_ps(score->map, i, MISS);
	else
	    trm_set_tro_ps(score->map, i, GOOD);

	if(OPT_SCORE_QUICK == 0 || trs_is_finished(score)) {
	    if(OPT_SCORE_QUICK)      
		trm_pattern_free(score->map);

	    trm_compute_stars(score->map);
	    trs_print_and_db(score);
	}
    }
}

//--------------------------------------------------

static void trs_print(struct tr_score * score)
{
    fprintf(OUTPUT_INFO, "Score: %.5g%% \t(aim: %.4g%%) [%d|%d|%d] (%d/%d)\n",
	    score->map->acc * COEFF_MAX_ACC, 
	    score->acc * COEFF_MAX_ACC, 
	    score->map->great, score->map->good, score->map->miss,
	    score->map->combo, score->map->max_combo);  
    trm_print(score->map);
}

//--------------------------------------------------
/*
  #include "replay/replay.h"
  #include "mod/game_mode.h"
  #include "mod/mods.h"

  #define CONVERT_MOD(RP_MOD, TR_MOD, rp_mods, mods)	\
  if((rp_mods & RP_MOD) != 0)				\
  mods |= TR_MOD

  void trs_main_replay(char * replay_file_name, struct tr_map * map)
  {
  FILE * f = fopen(replay_file_name, "r");
  struct replay * replay = replay_parse(f);

  if(replay->game_mode != MODE_TAIKO)
  {
  tr_error("Not a taiko score.");
  replay_free(replay);
  return;
  }

  struct tr_score * score = trs_new(map);

  int mods = MODS_NONE;
  CONVERT_MOD(MOD_EASY,       MODS_EZ, replay->mods, mods);
  CONVERT_MOD(MOD_HARDROCK,   MODS_HR, replay->mods, mods);
  CONVERT_MOD(MOD_HIDDEN,     MODS_HD, replay->mods, mods);
  CONVERT_MOD(MOD_FLASHLIGHT, MODS_FL, replay->mods, mods);
  CONVERT_MOD(MOD_DOUBLETIME, MODS_DT, replay->mods, mods);
  CONVERT_MOD(MOD_HALFTIME,   MODS_HT, replay->mods, mods);

  score->map = trm_copy(score->origin);
  trm_set_mods(score->map, mods);

  score->great = replay->_300;
  score->good  = replay->_100;
  score->miss  = replay->_miss;
  score->acc   = compute_acc(score->great, score->good, score->miss);
  
  replay_free(replay);
  
  //trs_compute(score);
  trs_free(score);
  }
*/
//--------------------------------------------------
