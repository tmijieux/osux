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

#ifndef OSUX_YAML_H
#define OSUX_YAML_H

#include <glib.h>

typedef enum osux_yaml_type_ {
    OSUX_YAML_INVALID = 0,
    OSUX_YAML_LIST,
    OSUX_YAML_TABLE,
    OSUX_YAML_SCALAR,
} osux_yaml_type;

typedef struct osux_yaml_ {
    osux_yaml_type type;
    union {
        GList *list;
        GHashTable *table;
        gchar *scalar;
        gpointer value;
    };
} osux_yaml;

osux_yaml *osux_yaml_new_from_file(char const *file_name);
void osux_yaml_dump(FILE *out, osux_yaml const *yaml);
void osux_yaml_free(osux_yaml *yaml);

#endif // OSUX_YAML_H
