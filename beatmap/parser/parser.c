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

#include <dlfcn.h>
#include "beatmap/beatmap.h"
#include "util/data.h"
#include "util/error.h"

osux_beatmap* (*osux_parse_beatmap)(const char*) = NULL;

__attribute__((constructor)) 
static void parser_init(void)
{
    void *handle;
    handle = dlopen(PKG_LIB_DIR"/libosux_pyparser.so", RTLD_LOCAL|RTLD_NOW);
    if (NULL == handle) {
        osux_error("Failed to initialize parser:\n%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
}
