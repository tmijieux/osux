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
#include <gmodule.h>

#include "osux/compiler.h"
#include "osux/beatmap.h"
#include "osux/data.h"
#include "osux/error.h"
#include "osux/parser.h"

extern const osux_beatmap DEFAULT_BEATMAP;

static struct osux_bm_parser_callback callbacks = {
    NULL
};

void osux_register_bm_callback(struct osux_bm_parser_callback *cb)
{
    if (cb != NULL)
        callbacks = *cb;
}

osux_parser_t osux_get_parser(void)
{
    return callbacks.parse_beatmap;
}

INITIALIZER(parser_init)
{
    plugin_init_t parser_py_init;
		
    if (!g_module_supported()){
        fprintf(stderr, "dynamic library loading is not supported'(\n");
        exit(EXIT_FAILURE);
    }
    gchar *fullPath = g_module_build_path(MODULE_LOAD_PATH, "osux_parser_py");
    GModule *pythonModule = g_module_open(fullPath, G_MODULE_BIND_LOCAL);

    if (pythonModule == NULL) {
        fprintf(stderr, "cannot load %s\n", fullPath);
        exit(EXIT_FAILURE);
    }
	g_free(fullPath);
	
    if (!g_module_symbol(pythonModule, "parser_py_init", (gpointer) &parser_py_init)) {
        fprintf(stderr, "cannot find function \"parser_py_init\"\n");
        exit(EXIT_FAILURE);
    }

    parser_py_init(&osux_register_bm_callback, &DEFAULT_BEATMAP);
}
