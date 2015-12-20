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

#include "util/hashtable/hash_table.h"
#include "util/list/list.h"
#include "yaml/yaml2.h"

#include "taiko_ranking_map.h"
#include "stats.h"
#include "cst_yaml.h"
#include "print.h"

//--------------------------------------------------

struct hash_table * cst_get_ht(char * file_name)
{
  struct yaml_wrap * yw = NULL;
  if(0 != yaml2_parse_file(&yw, file_name))
    tr_error("Unable to parse yaml file.");
  else if(yw->type != YAML_MAPPING)
    tr_error("Yaml file does not begin with a mapping.");
  else
    return yw->content.mapping;
  return NULL;
}

//--------------------------------------------------

double cst_f(struct hash_table * ht, const char * key)
{
  struct yaml_wrap * yw = NULL;
  ht_get_entry(ht, key, &yw);
  if(yw == NULL)
    tr_error("Constant %s not found.", key);
  else if(yw->type != YAML_SCALAR)
    tr_error("Constant %s is not a scalar.", key);
  else
    return atof(yw->content.scalar);
  return -1; 
}

//--------------------------------------------------

struct stats * cst_stats(struct hash_table * ht, const char * key)
{
  struct stats * stats = calloc(sizeof(*stats), 1);
  struct yaml_wrap * yw = NULL;
  ht_get_entry(ht, key, &yw);
  if(yw == NULL)
    tr_error("Stats %s not found.", key);
  else if(yw->type != YAML_MAPPING)
    tr_error("Stats %s is not a mapping.", key);
  else
    {
      struct hash_table * ht_stats = yw->content.mapping;
      stats->scaling = cst_f(ht_stats, "scaling");
      stats->median  = cst_f(ht_stats, "median");
      stats->mean    = cst_f(ht_stats, "mean");
      stats->d1 = cst_f(ht_stats, "d1");
      stats->d9 = cst_f(ht_stats, "d9");
      stats->q1 = cst_f(ht_stats, "q1");
      stats->q3 = cst_f(ht_stats, "q3");
    }
  return stats;
}
