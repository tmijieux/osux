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

#ifndef PARSER_H
#define PARSER_H

#include "beatmap/beatmap.h"

typedef osux_beatmap *(*osux_parser_t)(const char *filename);

struct osux_bm_parser_callback {
	osux_parser_t parse_beatmap;
};

// void osux_register_bm_callback(struct osux_bm_parser_callback *cb);
osux_parser_t osux_get_parser(void);

#endif //PARSER_H
