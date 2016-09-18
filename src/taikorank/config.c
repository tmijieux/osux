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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osux.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_score.h"
#include "tr_db.h"
#include "tr_mods.h"
#include "cst_yaml.h"
#include "config.h"
#include "print.h"

#define CONFIG_FILE "config.yaml"

struct tr_global_config * GLOBAL_CONFIG;
struct tr_local_config * LOCAL_CONFIG;

static struct yaml_wrap * yw;
static osux_hashtable * ht_conf;

static void local_config_score(void);

//-----------------------------------------------------

static struct tr_global_config * tr_global_config_new(void)
{
    return g_malloc0(sizeof(struct tr_global_config));
}

void tr_global_config_free(struct tr_global_config *conf)
{
    g_free(conf);
}

void tr_global_config_print(const struct tr_global_config *conf)
{
    fprintf(OUTPUT_INFO, "autoconvert: %d\n", conf->autoconvert_enable);

    fprintf(OUTPUT_INFO, "db_enable: %d\n", conf->db_enable);
    fprintf(OUTPUT_INFO, "db_ip:     %s\n", conf->db_ip);
    fprintf(OUTPUT_INFO, "db_login:  %s\n", conf->db_login);
    fprintf(OUTPUT_INFO, "db_passwd: %s\n", conf->db_passwd);

    fprintf(OUTPUT_INFO, "print_tro:    %d\n", conf->print_tro);
    fprintf(OUTPUT_INFO, "print_yaml:   %d\n", conf->print_yaml);
    // print filter is not readable

    fprintf(OUTPUT_INFO, "print_order:  %s\n", conf->print_order);

    fprintf(OUTPUT_INFO, "bdb_enable: %d\n", conf->beatmap_db_enable);
    fprintf(OUTPUT_INFO, "bdb_path:   %s\n", conf->beatmap_db_path);
}

//-----------------------------------------------------

static struct tr_local_config * tr_local_config_new(void)
{
    return g_malloc0(sizeof(struct tr_local_config));
}

void tr_local_config_free(struct tr_local_config *conf)
{
    g_free(conf);
}

struct tr_local_config * tr_local_config_copy(void)
{
    struct tr_local_config * copy = tr_local_config_new();
    memcpy(copy, LOCAL_CONFIG, sizeof(*LOCAL_CONFIG));
    return copy;
}

//-----------------------------------------------------

static void config_init(void)
{
    GLOBAL_CONFIG = tr_global_config_new();
    LOCAL_CONFIG  = tr_local_config_new();

    GLOBAL_CONFIG->autoconvert_enable = cst_i(ht_conf, "autoconvert_enable");

    GLOBAL_CONFIG->print_tro   = cst_i(ht_conf, "print_tro");
    GLOBAL_CONFIG->print_yaml  = cst_i(ht_conf, "print_yaml");
    GLOBAL_CONFIG->print_order = cst_str(ht_conf, "print_order");
    global_config_set_filter(cst_str(ht_conf, "print_filter"));

    GLOBAL_CONFIG->db_enable = cst_i(ht_conf, "db_enable");
    GLOBAL_CONFIG->db_ip     = cst_str(ht_conf, "db_ip");
    GLOBAL_CONFIG->db_login  = cst_str(ht_conf, "db_login");
    GLOBAL_CONFIG->db_passwd = cst_str(ht_conf, "db_passwd");

    GLOBAL_CONFIG->beatmap_db_enable = cst_i(ht_conf, "osuxdb_enable");
    GLOBAL_CONFIG->beatmap_db_path   = cst_str(ht_conf, "osuxdb_path");

    LOCAL_CONFIG->flat     = cst_i(ht_conf, "flat");
    LOCAL_CONFIG->no_bonus = cst_i(ht_conf, "no_bonus");
    local_config_set_mods(cst_str(ht_conf, "mods"));
    local_config_score();
}

//-----------------------------------------------------

void local_config_set_tr_main(enum tr_main i)
{
    switch (i) {
    case MAIN_SCORE:
        LOCAL_CONFIG->tr_main = trs_main;
        break;
    default:
        LOCAL_CONFIG->tr_main = trm_main;
        break;
    }
}

void local_config_set_score_method(enum score_method i)
{
    switch (i) {
    case SCORE_INPUT_INFLUENCE:
        LOCAL_CONFIG->trm_method_get_tro = trm_best_influence_tro;
        break;
    default:
        LOCAL_CONFIG->trm_method_get_tro = trm_hardest_tro;
        break;
    }
}

static void local_config_score(void)
{
    local_config_set_tr_main(cst_i(ht_conf, "score"));

    LOCAL_CONFIG->quick = cst_i(ht_conf, "score_quick");
    LOCAL_CONFIG->step  = cst_f(ht_conf, "score_step");
    LOCAL_CONFIG->input = cst_i(ht_conf, "score_input");
    LOCAL_CONFIG->good  = cst_i(ht_conf, "score_good");
    LOCAL_CONFIG->miss  = cst_i(ht_conf, "score_miss");
    LOCAL_CONFIG->acc   = cst_f(ht_conf, "score_acc");
    local_config_set_score_method(cst_i(ht_conf, "score_method"));
}

//-----------------------------------------------------

#define CASE_FILTER(C, FILTER)                  \
    case C:                                     \
    GLOBAL_CONFIG->print_filter |= FILTER;      \
    break

void global_config_set_filter(const char * filter)
{
    GLOBAL_CONFIG->print_filter = 0;
    int i = 0;
    while (filter[i] != 0) {
        switch (filter[i]) {
            CASE_FILTER('b', FILTER_BASIC);
            CASE_FILTER('B', FILTER_BASIC_PLUS);
            CASE_FILTER('+', FILTER_ADDITIONNAL);
            CASE_FILTER('d', FILTER_DENSITY);
            CASE_FILTER('r', FILTER_READING);
            CASE_FILTER('R', FILTER_READING_PLUS);
            CASE_FILTER('p', FILTER_PATTERN);
            CASE_FILTER('a', FILTER_ACCURACY);
            CASE_FILTER('*', FILTER_STAR);
        default:
            break;
        }
        i++;
    }
}

//-----------------------------------------------------

void local_config_set_mods(const char * mods)
{
    LOCAL_CONFIG->mods = str_to_mods(mods);
}

//-----------------------------------------------------

static void config_exit(void)
{
    osux_beatmap_db_free(&GLOBAL_CONFIG->beatmap_db);
    tr_print_yaml_exit();
    tr_global_config_free(GLOBAL_CONFIG);
    tr_local_config_free(LOCAL_CONFIG);
    yaml2_free(yw);
}

void tr_config_initialize(void)
{
    yw = cst_get_yw(CONFIG_FILE);
    ht_conf = yw_extract_ht(yw);
    if (ht_conf == NULL) {
        tr_error("Unable to run without config.");
        exit(EXIT_FAILURE);
    }
    config_init();
    atexit(config_exit);
}

void init_enabled(void)
{
    if (GLOBAL_CONFIG->db_enable)
        tr_db_init();
    if (GLOBAL_CONFIG->beatmap_db_enable)
        osux_beatmap_db_init(&GLOBAL_CONFIG->beatmap_db,
                             GLOBAL_CONFIG->beatmap_db_path, ".", false);
}
