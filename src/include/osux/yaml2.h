#ifndef OSUX_YAML2_H
#define OSUX_YAML2_H

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

#include <glib.h>
#include "osux/list.h"
#include "osux/hash_table.h"

G_BEGIN_DECLS

enum yaml_type {
    YAML_INVALID = 0,
    YAML_MAPPING,
    YAML_SEQUENCE,
    YAML_SCALAR,
};

union yaml_content {
    osux_list *sequence;
    osux_hashtable *mapping;
    char *scalar;
    void *value;
};

struct yaml_wrap {
    enum yaml_type type;
    union yaml_content content;
};

int yaml2_parse_file(struct yaml_wrap **yamlw, char const *file_name);
void yaml2_dump(FILE *out, const struct yaml_wrap *yw);
void yaml2_free(struct yaml_wrap *yw);

G_END_DECLS

#endif // OSUX_YAML2_H
