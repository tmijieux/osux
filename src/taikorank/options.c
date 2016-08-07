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

#include "taiko_ranking_map.h"
#include "tr_db.h"
#include "config.h"
#include "print.h"
#include "options.h"
#include "osux.h"

#define ARG_OPT_AUTOCONVERT  "autoconvert"

#define ARG_OPT_DB           "db"

#define ARG_OPT_PRINT_TRO    "ptro"
#define ARG_OPT_PRINT_YAML   "pyaml"
#define ARG_OPT_PRINT_FILTER "pfilter"
#define ARG_OPT_PRINT_ORDER  "porder"

#define ARG_OPT_ODB          "odb"
#define ARG_OPT_ODB_PATH     "odb_path"

#define ARG_OPT_SCORE        "score"
#define ARG_OPT_SCORE_QUICK  "quick"
#define ARG_OPT_SCORE_INPUT  "input"
#define ARG_OPT_SCORE_GGM    "ggm"
#define ARG_OPT_SCORE_ACC    "acc"

#define ARG_OPT_MODS         "mods"
#define ARG_OPT_NO_BONUS     "no_bonus"
#define ARG_OPT_FLAT         "flat"

static struct hash_table * ht_local_opt;
static struct hash_table * ht_global_opt;

//-----------------------------------------------------

static int opt_set(int argc, const char ** argv, 
		       struct hash_table * ht_opt)
{
    int (* opt)(int, const char **) = NULL;
    ht_get_entry(ht_opt, argv[0], &opt);
    if (opt == NULL) {
	tr_error("Unknown option: '%s'.", argv[0]);
	return 0;
    }
    return opt(argc-1, (const char **) &argv[1]);
}

int local_opt_set(int argc, const char ** argv)
{
    return opt_set(argc, argv, ht_local_opt);
}

int global_opt_set(int argc, const char ** argv)
{
    return opt_set(argc, argv, ht_global_opt);
}

//-----------------------------------------------------

#define OPT_ARGC_ERR(argc, n, opt)				\
    if (argc < n) {						\
	tr_error("Option '%s' need %d arguments.", opt, n);	\
	return 0; 						\
    }

static int opt_db(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_DB);
    GLOBAL_CONFIG->db_enable = atoi(argv[0]);
    return 1;
}

//-----------------------------------------------------

static int opt_autoconvert(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_AUTOCONVERT);
    GLOBAL_CONFIG->autoconvert_enable = atoi(argv[0]);
    return 1;
}

//-----------------------------------------------------

static int opt_score(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_SCORE);
    local_config_set_tr_main(atoi(argv[0]));
    return 1;
}

static int opt_score_quick(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_SCORE_QUICK);
    local_config_set_tr_main(MAIN_SCORE);
    LOCAL_CONFIG->quick = atoi(argv[0]);
    return 1;
}

static int opt_score_input(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_SCORE_INPUT);
    local_config_set_tr_main(MAIN_SCORE);
    LOCAL_CONFIG->input = atoi(argv[0]);
    return 1;  
}

static int opt_score_acc(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_SCORE_ACC);
    local_config_set_tr_main(MAIN_SCORE);
    LOCAL_CONFIG->acc   = atof(argv[0]) / COEFF_MAX_ACC;
    LOCAL_CONFIG->input = SCORE_INPUT_ACC;
    return 1;
}

static int opt_score_ggm(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 2, ARG_OPT_SCORE_GGM);
    local_config_set_tr_main(MAIN_SCORE);
    LOCAL_CONFIG->good  = atoi(argv[0]);
    LOCAL_CONFIG->miss  = atoi(argv[1]);
    LOCAL_CONFIG->input = SCORE_INPUT_GGM;
    return 2;
}

//-----------------------------------------------------

static int opt_print_tro(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_PRINT_TRO);
    GLOBAL_CONFIG->print_tro = atoi(argv[0]);
    return 1;
}

static int opt_print_yaml(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_PRINT_YAML);
    GLOBAL_CONFIG->print_yaml = atoi(argv[0]);
    return 1;
}

static int opt_print_filter(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_PRINT_FILTER);
    global_config_set_filter(argv[0]);
    return 1;
}

static int opt_print_order(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_PRINT_ORDER);
    GLOBAL_CONFIG->print_order = (char *) argv[0];
    return 1;
}

//-----------------------------------------------------

static int opt_mods(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_MODS);
    local_config_set_mods(argv[0]);
    return 1;
}

static int opt_no_bonus(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_NO_BONUS);
    LOCAL_CONFIG->no_bonus = atoi(argv[0]);
    return 1;
}

static int opt_flat(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_FLAT);
    LOCAL_CONFIG->flat = atoi(argv[0]);
    return 1;
}

//-----------------------------------------------------

static int opt_odb(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_ODB);
    GLOBAL_CONFIG->odb_enable = atoi(argv[0]);
    return 1;    
}

static int opt_odb_path(int argc, const char ** argv)
{
    OPT_ARGC_ERR(argc, 1, ARG_OPT_ODB_PATH);
    GLOBAL_CONFIG->odb_path = (char*) argv[0];
    return 1;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

typedef int (*opt_fun)(int, const char**);

static inline 
void add_opt(struct hash_table * ht_opt, char * key, opt_fun f)
{
    ht_add_entry(ht_opt, key, f);
}

#define add_local_opt(KEY, FUNC)			\
    add_opt(ht_local_opt, LOCAL_OPT_PREFIX KEY, FUNC)

#define add_global_opt(KEY, FUNC)			\
    add_opt(ht_global_opt, GLOBAL_OPT_PREFIX KEY, FUNC)

//-----------------------------------------------------

static void options_exit(void)
{
    ht_free(ht_local_opt);
    ht_free(ht_global_opt);
}

INITIALIZER(options_init)
{
    // global options
    ht_global_opt = ht_create(0, NULL);

    add_global_opt(ARG_OPT_AUTOCONVERT,  opt_autoconvert);
    add_global_opt(ARG_OPT_DB,           opt_db);
    add_global_opt(ARG_OPT_ODB,          opt_odb);
    add_global_opt(ARG_OPT_ODB_PATH,     opt_odb_path);

    add_global_opt(ARG_OPT_PRINT_TRO,    opt_print_tro);
    add_global_opt(ARG_OPT_PRINT_YAML,   opt_print_yaml);
    add_global_opt(ARG_OPT_PRINT_ORDER,  opt_print_order);
    add_global_opt(ARG_OPT_PRINT_FILTER, opt_print_filter);

    // local options
    ht_local_opt  = ht_create(0, NULL);

    add_local_opt(ARG_OPT_MODS,         opt_mods);
    add_local_opt(ARG_OPT_NO_BONUS,     opt_no_bonus);
    add_local_opt(ARG_OPT_FLAT,         opt_flat);

    add_local_opt(ARG_OPT_SCORE,        opt_score);
    add_local_opt(ARG_OPT_SCORE_QUICK,  opt_score_quick);
    add_local_opt(ARG_OPT_SCORE_INPUT,  opt_score_input);
    add_local_opt(ARG_OPT_SCORE_ACC,    opt_score_acc);
    add_local_opt(ARG_OPT_SCORE_GGM,    opt_score_ggm);

    atexit(options_exit);
}

