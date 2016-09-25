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
#include "cst_yaml.h"
#include "print.h"

#define NO_SCALAR   "-1"
#define NO_SEQUENCE NULL
#define NO_MAPPING  NULL

//--------------------------------------------------

osux_yaml *cst_get_yw(const char *file_name)
{
    osux_yaml *yw;
    char *filepath = g_build_filename(PKG_CONFIG_DIR, file_name, NULL);
    yw = osux_yaml_new_from_file(filepath);
    g_free(filepath);
    if (yw == NULL)
        tr_error("No wrapping.");
    return yw;
}

//--------------------------------------------------

GHashTable *yw_extract_ht(osux_yaml *yw)
{
    if (yw->type != OSUX_YAML_TABLE) {
        tr_error("Constant is not a hash table.");
        return NO_MAPPING;
    }
    return yw->table;
}


GList *yw_extract_list(osux_yaml *yw)
{
    if (yw->type != OSUX_YAML_LIST) {
        tr_error("Constant is not a list.");
        return NO_SEQUENCE;
    }
    return yw->list;
}

char *yw_extract_scalar(osux_yaml *yw)
{
    if (yw->type != OSUX_YAML_SCALAR) {
        tr_error("Constant is not a scalar.");
        return NO_SCALAR;
    }
    return yw->scalar;
}

//--------------------------------------------------

GList *cst_list(GHashTable *ht, const char *key)
{
    osux_yaml * yw = NULL;
    yw = g_hash_table_lookup(ht, key);
    if (yw == NULL) {
        tr_error("List '%s' not found.", key);
        return NO_SEQUENCE;
    }
    return yw_extract_list(yw);
}

char *cst_str(GHashTable *ht, const char *key)
{
    osux_yaml * yw = NULL;
    yw = g_hash_table_lookup(ht, key);
    if (yw == NULL) {
        tr_error("Scalar '%s' not found.", key);
        return NO_SCALAR;
    }
    return yw_extract_scalar(yw);
}

double cst_f(GHashTable *ht, const char *key)
{
    return atof(cst_str(ht, key));
}

int cst_i(GHashTable *ht, const char *key)
{
    return atoi(cst_str(ht, key));
}
