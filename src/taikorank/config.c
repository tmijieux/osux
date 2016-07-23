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

#include "database/osux_db.h"
#include "util/data.h"

#include "util/hash_table.h"
#include "util/list.h"
#include "util/yaml2.h"

#include "compiler.h"
#include "taiko_ranking_map.h"
#include "taiko_ranking_score.h"
#include "tr_db.h"
#include "tr_mods.h"
#include "cst_yaml.h"
#include "config.h"
#include "print.h"

#define CONFIG_FILE  "config.yaml"

#define MOD_STR_LENGTH 2

int OPT_AUTOCONVERT;

int OPT_DATABASE;
int OPT_PRINT_TRO;
int OPT_PRINT_YAML;
int OPT_PRINT_FILTER;
char * OPT_PRINT_ORDER;

int OPT_FLAT;
int OPT_NO_BONUS;

char * TR_DB_IP;
char * TR_DB_LOGIN;
char * TR_DB_PASSWD;

char * OPT_ODB_PATH;
char * OPT_ODB_SGDIR;
char * OPT_ODB_STATE;
struct osux_db * ODB;

struct tr_config * CONF;

static struct yaml_wrap * yw;
static struct hash_table * ht_conf;

//-----------------------------------------------------

static struct tr_config * tr_config_new(void)
{
    return malloc(sizeof(struct tr_config));
}

void tr_config_free(struct tr_config * conf)
{
    free(conf);
}

struct tr_config * tr_config_copy(struct tr_config * conf)
{
    struct tr_config * copy = tr_config_new();
    memcpy(copy, conf, sizeof(*conf));
    return copy;
}

//-----------------------------------------------------

static void global_init(void)
{
    CONF = tr_config_new();

    OPT_AUTOCONVERT = cst_i(ht_conf, "enable_autoconvert");

    OPT_PRINT_TRO   = cst_i(ht_conf, "print_tro");
    OPT_PRINT_YAML  = cst_i(ht_conf, "print_yaml");
    OPT_PRINT_ORDER = cst_str(ht_conf, "print_order");
    config_set_filter(cst_str(ht_conf, "print_filter"));
  
    config_set_mods(cst_str(ht_conf, "mods"));
    OPT_FLAT     = cst_i(ht_conf, "flat");
    OPT_NO_BONUS = cst_i(ht_conf, "no_bonus");

    OPT_DATABASE = cst_i(ht_conf, "database");
    if(OPT_DATABASE)
	ht_conf_db_init();

    config_score();

    OPT_ODB_PATH  = cst_str(ht_conf, "osuxdb_path");
    OPT_ODB_STATE = cst_str(ht_conf, "osuxdb_state");
    OPT_ODB_SGDIR = cst_str(ht_conf, "osuxdb_song_dir");
    osux_set_song_path(OPT_ODB_SGDIR);
    config_odb_apply_state(OPT_ODB_STATE[0]);
}

//-----------------------------------------------------

void config_set_tr_main(int score)
{
    if (score)
	CONF->tr_main = trs_main;
    else
	CONF->tr_main = trm_main;
}

void config_score(void)
{
    config_set_tr_main(cst_i(ht_conf, "score"));

    CONF->quick = cst_i(ht_conf, "score_quick");
    CONF->input = cst_i(ht_conf, "score_input");
    CONF->good  = cst_i(ht_conf, "score_good");
    CONF->miss  = cst_i(ht_conf, "score_miss");
    CONF->acc   = cst_f(ht_conf, "score_acc") / COEFF_MAX_ACC;
    enum score_method i = cst_i(ht_conf, "score_method");
    switch(i) {
    case SCORE_INPUT_INFLUENCE:
	CONF->trm_method_get_tro = trm_best_influence_tro;
	break;
    default:
	CONF->trm_method_get_tro = trm_hardest_tro;
	break;
    }
}

//-----------------------------------------------------

static void config_odb_exit(void)
{
    if (ODB != NULL)
	osux_db_free(ODB);
}

static void config_odb_build(char * song_dir)
{
    osux_db_build(song_dir, &ODB);
    osux_db_save(OPT_ODB_PATH, ODB);
    atexit(config_odb_exit);
}

void config_odb_apply_state(char odb_state)
{
    switch (odb_state) {
    case 'b':
	config_odb_build(OPT_ODB_SGDIR);
	break;
    case 'l':
	osux_db_load(OPT_ODB_PATH, &ODB);
	break;
    default:
	break;
    }    
}

//-----------------------------------------------------

#define CASE_FILTER(C, FILTER)			\
    case C:					\
    OPT_PRINT_FILTER |= FILTER;			\
    break

void config_set_filter(char * filter)
{
    OPT_PRINT_FILTER = 0;
    int i = 0;
    while(filter[i] != 0) {
	switch(filter[i]) {
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

#define IF_MOD_SET(STR, MOD, i)				\
    if(strncmp(STR, &mods[i], MOD_STR_LENGTH) == 0) {	\
	CONF->mods |= MOD;				\
	continue;					\
    }

void config_set_mods(const char * mods)
{
    CONF->mods = MODS_NONE;
    for(int i = 0; mods[i]; i += MOD_STR_LENGTH) {
	IF_MOD_SET("EZ", MODS_EZ, i);
	IF_MOD_SET("HR", MODS_HR, i);
	IF_MOD_SET("HT", MODS_HT, i);
	IF_MOD_SET("DT", MODS_DT, i);
	IF_MOD_SET("HD", MODS_HD, i);
	IF_MOD_SET("FL", MODS_FL, i);
	IF_MOD_SET("__", MODS_NONE, i);
	for(int k = 1; k < MOD_STR_LENGTH; k++) {
	    if(mods[i+k] == 0) {
		tr_error("Wrong mod length.");
		goto break2;
	    }
	}
	tr_error("Unknown mod used.");
    }
 break2:
    
    if((CONF->mods & MODS_EZ) && (CONF->mods & MODS_HR))
	tr_error("Incompatible mods EZ and HR");
    if((CONF->mods & MODS_HT) && (CONF->mods & MODS_DT))
	tr_error("Incompatible mods HT and DT");
    if((CONF->mods & MODS_HD) && (CONF->mods & MODS_HR))
	tr_error("HDHR is unsupported for now.");
}

//-----------------------------------------------------

void ht_conf_db_init(void)
{
    TR_DB_IP     = cst_str(ht_conf, "db_ip");
    TR_DB_LOGIN  = cst_str(ht_conf, "db_login");
    TR_DB_PASSWD = cst_str(ht_conf, "db_passwd");
}

//-----------------------------------------------------


static void ht_cst_exit_config(void)
{
	tr_print_yaml_exit();
	tr_config_free(CONF);
	yaml2_free(yw);
}

INITIALIZER(ht_cst_init_config)
{
    yw = cst_get_yw(CONFIG_FILE);
    ht_conf = yw_extract_ht(yw);
    if(ht_conf == NULL) {
	tr_error("Unable to run without config.");
	exit(EXIT_FAILURE);
    }
    global_init();
    if (OPT_DATABASE)
	tr_db_init();
    atexit(ht_cst_exit_config);
}
