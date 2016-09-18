#ifndef OSUX_PARSER_H
#define OSUX_PARSER_H

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
#include "osux/beatmap.h"

G_BEGIN_DECLS

typedef osux_beatmap *(*osux_parser_t)(const char *filename);

struct osux_bm_parser_callback {
	osux_parser_t parse_beatmap;
};

typedef void (*register_plugin_t)(struct osux_bm_parser_callback*);
typedef void (*plugin_init_t)(register_plugin_t callback,
                              osux_beatmap const *default_map);

// void osux_register_bm_callback(struct osux_bm_parser_callback *cb);
osux_parser_t osux_get_parser(void);

G_END_DECLS

#endif // OSUX_PARSER_H
