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

#include "util/hashtable/hash_table.h"
#include "util/list/list.h"
#include "yaml/yaml2.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_score.h"
#include "tr_db.h"
#include "cst_yaml.h"
#include "config.h"

#define CONFIG_FILE  "config.yaml"

int OPT_DATABASE;
int OPT_PRINT_TRO;
int OPT_PRINT_YAML;
char * OPT_PRINT_ORDER;

char * TR_DB_IP;
char * TR_DB_LOGIN;
char * TR_DB_PASSWD;

int OPT_SCORE;
double SCORE_ACC;
int (* TRM_METHOD_GET_TRO)(struct tr_map *);

static struct yaml_wrap * yw;
static struct hash_table * ht_conf;

//-----------------------------------------------------

static void global_init(void)
{
  OPT_PRINT_TRO   = cst_i(ht_conf, "print_tro");
  OPT_PRINT_YAML  = cst_i(ht_conf, "print_yaml");
  OPT_PRINT_ORDER = cst_str(ht_conf, "print_order");

  OPT_DATABASE = cst_i(ht_conf, "database");
  if(OPT_DATABASE)
    ht_conf_db_init();

  OPT_SCORE = cst_i(ht_conf, "score");
  if(OPT_SCORE)
    {
      SCORE_ACC = cst_f(ht_conf, "score_acc");
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
  yaml2_free(yw);
}
