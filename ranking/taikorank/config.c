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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/hash_table.h"
#include "util/list.h"
#include "util/yaml2.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_score.h"
#include "tr_db.h"
#include "mods.h"
#include "cst_yaml.h"
#include "config.h"
#include "print.h"

#define CONFIG_FILE  "config.yaml"

#define MOD_STR_LENGTH 2

int OPT_DATABASE;
int OPT_PRINT_TRO;
int OPT_PRINT_YAML;
int OPT_PRINT_FILTER;
char * OPT_PRINT_ORDER;

int OPT_MODS;

char * TR_DB_IP;
char * TR_DB_LOGIN;
char * TR_DB_PASSWD;

int OPT_SCORE;
int OPT_SCORE_QUICK;
double OPT_SCORE_ACC;
int (* TRM_METHOD_GET_TRO)(struct tr_map *);

static struct yaml_wrap * yw;
static struct hash_table * ht_conf;

static void config_set_filter(void);
static void config_set_mods(void);

//-----------------------------------------------------

static void global_init(void)
{
  OPT_PRINT_TRO    = cst_i(ht_conf, "print_tro");
  OPT_PRINT_YAML   = cst_i(ht_conf, "print_yaml");
  OPT_PRINT_ORDER  = cst_str(ht_conf, "print_order");
  config_set_filter();
  config_set_mods();

  OPT_DATABASE = cst_i(ht_conf, "database");
  if(OPT_DATABASE)
    ht_conf_db_init();

  OPT_SCORE = cst_i(ht_conf, "score");
  if(OPT_SCORE)
    {
      OPT_SCORE_QUICK = cst_i(ht_conf, "score_quick");
      OPT_SCORE_ACC   = cst_f(ht_conf, "score_acc") / COEFF_MAX_ACC;
      int i = cst_i(ht_conf, "score_method");
      switch(i)
	{
	case 1:
	  TRM_METHOD_GET_TRO = trm_best_influence_tro;
	  break;
	default:
	  TRM_METHOD_GET_TRO = trm_hardest_tro;
	  break;
	}
    }
}

//-----------------------------------------------------

#define CASE_FILTER(C, FILTER)			\
  case C:					\
  OPT_PRINT_FILTER |= FILTER;			\
  break

static void config_set_filter(void)
{
  OPT_PRINT_FILTER = 0;
  char * filter = cst_str(ht_conf, "print_filter");
  int i = 0;
  while(filter[i] != 0)
    {
      switch(filter[i])
	{
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
  if(strncmp(STR, &mods[i], MOD_STR_LENGTH) == 0)	\
    {							\
      OPT_MODS |= MOD;					\
      continue;						\
    }

static void config_set_mods(void)
{
  OPT_MODS = MODS_NONE;
  char * mods = cst_str(ht_conf, "mods");
  for(int i = 0; mods[i]; i += MOD_STR_LENGTH)
    {
      IF_MOD_SET("EZ", MODS_EZ, i);
      IF_MOD_SET("HR", MODS_HR, i);
      IF_MOD_SET("HT", MODS_HT, i);
      IF_MOD_SET("DT", MODS_DT, i);
      IF_MOD_SET("HD", MODS_HD, i);
      IF_MOD_SET("FL", MODS_FL, i);
    }
  if((OPT_MODS & MODS_EZ) && (OPT_MODS & MODS_HR))
    tr_error("Incompatible mods EZ and HR");
  if((OPT_MODS & MODS_HT) && (OPT_MODS & MODS_DT))
    tr_error("Incompatible mods HT and DT");
  if((OPT_MODS & MODS_HD) && (OPT_MODS & MODS_HR))
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

__attribute__((constructor))
static void ht_cst_init_config(void)
{
  yw = cst_get_yw(CONFIG_FILE);
  ht_conf = cst_get_ht(yw);
  if(ht_conf == NULL)
    {
      tr_error("Unable to run without config.");
      exit(EXIT_FAILURE);
    }
  global_init();
  if(OPT_DATABASE)
    tr_db_init();
  if(OPT_PRINT_YAML)
    tr_print_yaml_init();
}

__attribute__((destructor))
static void ht_cst_exit_config(void)
{
  if(OPT_PRINT_YAML)
    tr_print_yaml_exit();
  if(yw)
    yaml2_free(yw);
}
