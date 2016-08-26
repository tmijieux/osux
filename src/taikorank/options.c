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

typedef void (*tr_option_fun)(const char **);

enum tr_option_type
{
    local_opt,
    global_opt,
};

struct tr_option
{
    const char * short_key;
    const char * long_key;
    int nb_arg;
    enum tr_option_type type;
    tr_option_fun fun;
    const char * help;
};

static osux_hashtable * ht_local_opt;
static osux_hashtable * ht_global_opt;

//-----------------------------------------------------

static int tr_option_apply(const struct tr_option * opt,
			   int argc, const char ** argv)
{
    if (argc < opt->nb_arg) {
	tr_error("Option '%s' need %d arguments.",
		 opt->long_key, opt->nb_arg);
	return 0;
    }
    opt->fun(argv);
    return opt->nb_arg;
}

//-----------------------------------------------------

static int opt_set(int argc, const char ** argv,
		   osux_hashtable * ht_opt)
{
    struct tr_option * opt = NULL;
    osux_hashtable_lookup(ht_opt, argv[0], &opt);
    if (opt == NULL) {
	tr_error("Unknown option: '%s'", argv[0]);
	return 0;
    }
    return tr_option_apply(opt, argc-1, (const char **) &argv[1]);
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

static void opt_db(const char ** argv)
{
    GLOBAL_CONFIG->db_enable = atoi(argv[0]);
}

//-----------------------------------------------------

static void opt_autoconvert(const char ** argv)
{
    GLOBAL_CONFIG->autoconvert_enable = atoi(argv[0]);
}

//-----------------------------------------------------

static void opt_score(const char ** argv)
{
    local_config_set_tr_main(atoi(argv[0]));
}

static void opt_score_quick(const char ** argv)
{
    local_config_set_tr_main(MAIN_SCORE);
    LOCAL_CONFIG->quick = atoi(argv[0]);
}

static void opt_score_input(const char ** argv)
{
    local_config_set_tr_main(MAIN_SCORE);
    LOCAL_CONFIG->input = atoi(argv[0]);
}

static void opt_score_acc(const char ** argv)
{
    local_config_set_tr_main(MAIN_SCORE);
    LOCAL_CONFIG->acc   = atof(argv[0]) / COEFF_MAX_ACC;
    LOCAL_CONFIG->input = SCORE_INPUT_ACC;
}

static void opt_score_ggm(const char ** argv)
{
    local_config_set_tr_main(MAIN_SCORE);
    LOCAL_CONFIG->good  = atoi(argv[0]);
    LOCAL_CONFIG->miss  = atoi(argv[1]);
    LOCAL_CONFIG->input = SCORE_INPUT_GGM;
}

//-----------------------------------------------------

static void opt_print_tro(const char ** argv)
{
    GLOBAL_CONFIG->print_tro = atoi(argv[0]);
}

static void opt_print_yaml(const char ** argv)
{
    GLOBAL_CONFIG->print_yaml = atoi(argv[0]);
}

static void opt_print_filter(const char ** argv)
{
    global_config_set_filter(argv[0]);
}

static void opt_print_order(const char ** argv)
{
    GLOBAL_CONFIG->print_order = (char *) argv[0];
}

//-----------------------------------------------------

static void opt_mods(const char ** argv)
{
    local_config_set_mods(argv[0]);
}

static void opt_no_bonus(const char ** argv)
{
    LOCAL_CONFIG->no_bonus = atoi(argv[0]);
}

static void opt_flat(const char ** argv)
{
    LOCAL_CONFIG->flat = atoi(argv[0]);
}

//-----------------------------------------------------

static void opt_bdb(const char ** argv)
{
    GLOBAL_CONFIG->beatmap_db_enable = atoi(argv[0]);
}

static void opt_bdb_path(const char ** argv)
{
    GLOBAL_CONFIG->beatmap_db_path = (char*) argv[0];
}

//-----------------------------------------------------

static void tr_option_print(const char * key UNUSED,
			    struct tr_option * opt)
{
    fprintf(OUTPUT_INFO, "\t%s\t%s\n", opt->long_key, opt->help);
}

static void opt_help(const char ** argv UNUSED)
{
    fprintf(OUTPUT_INFO, "Usage:\n");
    fprintf(OUTPUT_INFO, "taiko_ranking [GLOBAL_OPTION] ... "
	    "[LOCAL_OPTIONS] [FILE|HASH] ...\n");
    fprintf(OUTPUT_INFO, "Global options:\n");
    osux_hashtable_each(ht_global_opt, (void (*)(const char*, void*))
			tr_option_print);
    fprintf(OUTPUT_INFO, "Local options:\n");
    osux_hashtable_each(ht_local_opt, (void (*)(const char*, void*))
			tr_option_print);
    exit(EXIT_SUCCESS);
}

void print_help(void)
{
    opt_help(NULL);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

typedef int (*opt_fun)(int, const char**);

static inline
void add_opt(osux_hashtable * ht_opt, struct tr_option * opt)
{
    osux_hashtable_insert(ht_opt, opt->long_key, opt);
    if (opt->short_key != NULL)
	osux_hashtable_insert(ht_opt, opt->short_key, opt);
}

#define new_tr_local_opt(LG_KEY, NB_ARG, FUNC, HELP)		\
    static struct tr_option tr_opt_##FUNC = {			\
	NULL,							\
	LOCAL_OPT_PREFIX LG_KEY,				\
	NB_ARG, local_opt, FUNC, HELP				\
    };								\
    add_opt(ht_local_opt, &tr_opt_##FUNC)

#define new_tr_global_opt(LG_KEY, NB_ARG, FUNC, HELP)		\
    static struct tr_option tr_opt_##FUNC = {			\
	NULL,							\
	GLOBAL_OPT_PREFIX LG_KEY,				\
	NB_ARG, global_opt, FUNC, HELP				\
    };								\
    add_opt(ht_global_opt, &tr_opt_##FUNC)


//-----------------------------------------------------

static void options_exit(void)
{
    osux_hashtable_delete(ht_local_opt);
    osux_hashtable_delete(ht_global_opt);
}

void tr_options_initialize(void)
{
    // global options
    ht_global_opt = osux_hashtable_new(0);

    new_tr_global_opt("help", 0, opt_help,
		      "Show this message");

    new_tr_global_opt("autoconvert", 1, opt_autoconvert,
		      "Enable or disable autoconvertion");
    new_tr_global_opt("db", 1, opt_db,
		      "Enable or disable database storing");
    new_tr_global_opt("bdb", 1, opt_bdb,
		      "Enable or disable beatmap database lookup");
    new_tr_global_opt("bdb_path", 1, opt_bdb_path,
		      "Set the path to the beatmap database");

    new_tr_global_opt("ptro", 1, opt_print_tro,
		      "Enable or disable object printing");
    new_tr_global_opt("pyaml", 1, opt_print_yaml,
		      "Enable or disable yaml output");
    new_tr_global_opt("porder", 1, opt_print_order,
		      "Set star order");
    new_tr_global_opt("pfilter", 1, opt_print_filter,
		      "Set printed data filter");

    // local options
    ht_local_opt  = osux_hashtable_new(0);

    new_tr_local_opt("mods", 1, opt_mods,
		     "Set mods");
    new_tr_local_opt("no_bonus", 1, opt_no_bonus,
		     "Enable or disable bonus objects removing");
    new_tr_local_opt("flat", 1, opt_flat,
		     "Enable or disable objects flatening");

    new_tr_local_opt("score", 1, opt_score,
		     "Enable or disable score computation");
    new_tr_local_opt("quick", 1, opt_score_quick,
		     "Enable or disable quick score computation");
    new_tr_local_opt("input", 1, opt_score_input,
		     "Set score input method");
    new_tr_local_opt("acc", 1, opt_score_acc,
		     "Set score accuracy");
    new_tr_local_opt("ggm", 2, opt_score_ggm,
		     "Set score number of good and miss");

    atexit(options_exit);
}
