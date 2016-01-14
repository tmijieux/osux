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

#include "util/hash_table.h"

#define ARG_OPT_DB           "-db"
#define ARG_OPT_SCORE_ACC    "-acc"
#define ARG_OPT_SCORE_QUICK  "-quick"
#define ARG_OPT_PRINT_TRO    "-ptro"
#define ARG_OPT_PRINT_YAML   "-pyaml"
#define ARG_OPT_PRINT_FILTER "-pfilter"
#define ARG_OPT_PRINT_ORDER  "-porder"
#define ARG_OPT_MODS         "-mods"

static struct hash_table * ht_opt;

//-----------------------------------------------------

int options_set(int argc, const char ** argv)
{
  int (* opt)(int, const char **) = NULL;
  ht_get_entry(ht_opt, argv[0], &opt);
  if(opt == NULL)
    {
      tr_error("Unknown option: '%s'.", argv[0]);
      return 0;
    }
  return opt(argc-1, (const char **) &argv[1]);
}

//-----------------------------------------------------

#define OPT_ARGC_ERR(argc, n, opt)				\
  if(argc < n)							\
    {								\
      tr_error("Option '%s' need %d arguments.", opt, n);	\
      return 0; 						\
    }

static int opt_db(int argc, const char ** argv)
{
  OPT_ARGC_ERR(argc, 1, ARG_OPT_DB);
  OPT_DATABASE = atoi(argv[0]);
  if(OPT_DATABASE)
    {
      ht_conf_db_init();
      tr_db_init();
    }
  return 1;
}

static int opt_score_quick(int argc, const char ** argv)
{
  OPT_ARGC_ERR(argc, 1, ARG_OPT_SCORE_QUICK);
  OPT_SCORE_QUICK = atoi(argv[0]);
  return 1;
}

static int opt_score_acc(int argc, const char ** argv)
{
  OPT_ARGC_ERR(argc, 1, ARG_OPT_SCORE_ACC);
  OPT_SCORE_ACC = atof(argv[0]) / COEFF_MAX_ACC;
  return 1;
}

static int opt_print_tro(int argc, const char ** argv)
{
  OPT_ARGC_ERR(argc, 1, ARG_OPT_PRINT_TRO);
  OPT_PRINT_TRO = atoi(argv[0]);
  return 1;
}

static int opt_print_yaml(int argc, const char ** argv)
{
  OPT_ARGC_ERR(argc, 1, ARG_OPT_PRINT_YAML);
  OPT_PRINT_YAML = atoi(argv[0]);
  return 1;
}

static int opt_print_filter(int argc, const char ** argv)
{
  OPT_ARGC_ERR(argc, 1, ARG_OPT_PRINT_FILTER);
  return 1;
}

static int opt_print_order(int argc, const char ** argv)
{
  OPT_ARGC_ERR(argc, 1, ARG_OPT_PRINT_ORDER);
  OPT_PRINT_ORDER = (char *) argv[0];
  return 1;
}

static int opt_mods(int argc, const char ** argv)
{
  OPT_ARGC_ERR(argc, 1, ARG_OPT_MODS);
  config_set_mods(argv[0]);
  return 1;
}

//-----------------------------------------------------

__attribute__ ((constructor))
static void options_init(void)
{
  ht_opt = ht_create(0, NULL);
  
  ht_add_entry(ht_opt, ARG_OPT_DB,           opt_db);
  ht_add_entry(ht_opt, ARG_OPT_MODS,         opt_mods);
  ht_add_entry(ht_opt, ARG_OPT_SCORE_ACC,    opt_score_acc);
  ht_add_entry(ht_opt, ARG_OPT_SCORE_QUICK,  opt_score_quick);
  ht_add_entry(ht_opt, ARG_OPT_PRINT_TRO,    opt_print_tro);
  ht_add_entry(ht_opt, ARG_OPT_PRINT_YAML,   opt_print_yaml);
  ht_add_entry(ht_opt, ARG_OPT_PRINT_ORDER,  opt_print_order);
  ht_add_entry(ht_opt, ARG_OPT_PRINT_FILTER, opt_print_filter);
}

__attribute__ ((destructor))
static void options_exit(void)
{
  ht_free(ht_opt);
}
