#ifndef OSUX_PARSER_CFETCH_H
#define OSUX_PARSER_CFETCH_H

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

osux_beatmap *osux_c_parse_beatmap(char const *filename);

G_END_DECLS

#endif // OSUX_PARSER_CFETCH_H
