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

#include "taiko_ranking_map.h"
#include "taiko_ranking_score.h"
#include "tr_db.h"
#include "config.h"
#include "print.h"

#include "util/hashtable/hash_table.h"

#define ARG_OPT_DB "-db"

static struct hash_table * ht_opt;

//-----------------------------------------------------

void options_set(const char * s, const char * arg)
{
  void (* opt)(const char *) = NULL;
  ht_get_entry(ht_opt, s, &opt);
  if(opt == NULL)
    tr_error("Unknown option: '%s'.", s);
  else
    opt(arg);
}

//-----------------------------------------------------

static void opt_db(const char * value)
{
  OPT_DATABASE = atoi(value);
  ht_conf_db_init();
  tr_db_init();
}

//-----------------------------------------------------

__attribute__ ((constructor))
static void options_init(void)
{
  ht_opt = ht_create(0, NULL);
  
  ht_add_entry(ht_opt, ARG_OPT_DB, opt_db);
}

__attribute__ ((destructor))
static void options_exit(void)
{
  ht_free(ht_opt);
}
