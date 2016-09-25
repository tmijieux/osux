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
#ifndef TR_CST_YAML_H
#define TR_CST_YAML_H

#include "osux.h"

osux_yaml *cst_get_yw(const char *file_name);
GHashTable *yw_extract_ht(osux_yaml *yw);
GList *yw_extract_list(osux_yaml *yw);
char *yw_extract_scalar(osux_yaml *yw);
GList *cst_list(GHashTable *ht, const char *key);
double cst_f(GHashTable *ht, const char *key);
int cst_i(GHashTable *ht, const char *key);
char *cst_str(GHashTable *ht, const char *key);

#endif // TR_CST_YAML_H
