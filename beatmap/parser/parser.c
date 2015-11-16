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

#include "pyparser/pyfetch.h"

static struct map* (*parser)(const char*) =
    &osux_py_parse_beatmap;

struct map *osux_parse_beatmap(const char *filename)
{
    return parser(filename);

    // TODO
    // get function pointer of any loaded module that provides this function
    // (either c or python)
}

struct map *osu_map_parser(const char *filename) // COMPATIBILITY
{
    return osux_parse_beatmap(filename);
}