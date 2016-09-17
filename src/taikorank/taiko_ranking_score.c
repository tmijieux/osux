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

#include "taiko_ranking_object.h"
#include "taiko_ranking_map.h"
#include "taiko_ranking_score.h"
#include "final_star.h"
#include "compute_stars.h"

#include "config.h"
#include "print.h"
#include "tr_db.h"
#include "tr_mods.h"

static void trs_print_and_db(const struct tr_score * score);
static void trs_compute(struct tr_score * score);

static struct tr_score * trs_new(const struct tr_map * map);
static void trs_free(struct tr_score * score);

//--------------------------------------------------

void trs_main(const struct tr_map * map)
{
    struct tr_score * score = trs_new(map);
    score->map = trm_copy(score->origin);
    trm_set_read_only_objects(score->map);
    trm_set_mods(score->map, map->conf->mods);

    // modifications
    trm_add_modifier(score->map);

    score->trs_prepare(score);
    trs_compute(score);
    trs_free(score);
}

//--------------------------------------------------

static void trs_prepare_ggm(struct tr_score * sc)
{
    int good = sc->origin->conf->good;
    int miss = sc->origin->conf->miss;
    if (good < 0)
        good = 0;
    if (miss < 0)
        miss = 0;

    // Remove exceeding good and miss
    while (good + miss > sc->origin->great) {
        if (good > 0)
            good--;
        else
            miss--;
    }

    sc->great = sc->origin->great - good - miss;
    sc->good  = good;
    sc->miss  = miss;
    sc->acc   = compute_acc(sc->great, sc->good, sc->miss);
}

static void trs_prepare_acc(struct tr_score * sc)
{
    double acc = sc->origin->conf->acc;
    if (acc < 0)
        acc = 0;
    else if (acc > MAX_ACC)
        acc = MAX_ACC;

    sc->great = sc->origin->great;
    sc->good  = sc->origin->good;
    sc->miss  = sc->origin->miss;
    sc->acc   = compute_acc(sc->great, sc->good, sc->miss);
    while (sc->acc > acc) {
        double try = compute_acc(sc->great-1, sc->good+1, sc->miss);
        if (try <= acc) {
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

static int trs_has_reached_step_ggm(struct tr_score * score)
{
    int total = score->map->good + score->map->miss;
    if (score->last_point + score->step <= total) {
        score->last_point = total;
        return 1;
    }
    return 0;
}

static int trs_has_reached_step_acc(struct tr_score * score)
{
    if (score->map->acc + score->step <= score->last_point) {
        score->last_point = score->map->acc;
        return 1;
    }
    return 0;
}

//--------------------------------------------------

static struct tr_score * trs_new(const struct tr_map * map)
{
    struct tr_score * sc = malloc(sizeof(*sc));
    sc->origin = map;
    if (map->conf->step < 0)
        sc->step = INFINITY;
    else
        sc->step = map->conf->step;
    switch (sc->origin->conf->input) {
    case SCORE_INPUT_ACC:
        sc->last_point = MAX_ACC;
        sc->trs_prepare = trs_prepare_acc;
        sc->trs_has_reached_step = trs_has_reached_step_acc;
        break;
    case SCORE_INPUT_GGM:
        sc->last_point = map->good + map->miss;
        sc->trs_prepare = trs_prepare_ggm;
        sc->trs_has_reached_step = trs_has_reached_step_ggm;
        break;
    default:
        tr_error("Wrong score input method.");
        break;
    }
    return sc;
}

//--------------------------------------------------

static void trs_free(struct tr_score * score)
{
    if (score == NULL)
        return;
    trm_free(score->map);
    free(score);
}

//--------------------------------------------------

static void trs_print_and_db(const struct tr_score * score)
{
    #pragma omp critical
    trs_print(score);

    if (GLOBAL_CONFIG->db_enable)
        trm_db_insert(score->map);
}

//--------------------------------------------------

static int trs_is_finished(const struct tr_score * score)
{
    return ((score->great == score->map->great) &&
            (score->good  == score->map->good) &&
            (score->miss  == score->map->miss));
}

//--------------------------------------------------

static int trs_change_one_object(struct tr_score * score)
{
    int i = score->map->conf->trm_method_get_tro(score->map);
    if (score->miss != score->map->miss)
        trm_set_tro_ps(score->map, i, MISS);
    else
        trm_set_tro_ps(score->map, i, GOOD);
    return i;
}

static int trs_compute_if_needed(struct tr_score * score, int i)
{
    /*
     * Without quick the map is recomputed everytime
     * Else the changed object influence is applied, this avoid to only
     * change the objects in the hardest time.
     */
    if (score->map->conf->quick == 0 || trs_is_finished(score)) {
        trm_compute_stars(score->map);
        return 1;
    } else {
        tro_set_influence(score->map->object, i,
                          score->map->nb_object);
    }
    return 0;
}

static void trs_print_and_db_if_needed(struct tr_score * score, int computed)
{
    if (score->trs_has_reached_step(score) || trs_is_finished(score)) {
        /*
         * The map is computed for printing but not reused. This ensure
         * the star rating won't change depending on the step when the quick
         * computation is used.
         */
        struct tr_map * saved = score->map;
        if (!computed) {
            score->map = trm_copy(score->map);
            trm_compute_stars(score->map);
        }
        trs_print_and_db(score);
        score->map = saved;
    }
}

//--------------------------------------------------

static void trs_compute(struct tr_score * score)
{
    trm_apply_mods(score->map);
    trm_compute_stars(score->map);
    if (score->step != INFINITY || trs_is_finished(score))
        trs_print_and_db(score);

    while (!trs_is_finished(score)) {
        int i = trs_change_one_object(score);
        int computed = trs_compute_if_needed(score, i);
        trs_print_and_db_if_needed(score, computed);
    }
}

//--------------------------------------------------

static void trs_print_out(const struct tr_score * score)
{
    fprintf(OUTPUT_INFO, "Score: %.5g%% \t(aim: %.4g%%) [%d|%d|%d] (%d/%d)\n",
            score->map->acc, score->acc,
            score->map->great, score->map->good, score->map->miss,
            score->map->combo, score->map->max_combo);
    trm_print(score->map);
}

void trs_print(const struct tr_score * score)
{
    if (GLOBAL_CONFIG->print_yaml)
        trm_print_yaml(score->map);
    else
        trs_print_out(score);
}
