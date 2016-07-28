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

#include "compiler.h"
#include "beatmap/beatmap.h"
#include "util/data.h"
#include "util/error.h"
#include "./parser.h"

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
		fprintf(stderr, "1 cannot load PYTHON ;'(\n");
		exit(EXIT_FAILURE);
	}
	GModule *pythonModule = g_module_open("python2.7", G_MODULE_BIND_LOCAL);
	if (pythonModule == NULL) {
		fprintf(stderr, "2 cannot load PYTHON 2;'(\n");
		exit(EXIT_FAILURE);
	}
	if (!g_module_symbol(pythonModule, "", (gpointer) &parser_py_init)) {
		fprintf(stderr, "3 cannot load PYTHON ;'(\n");
		exit(EXIT_FAILURE);
	}

	parser_py_init(&osux_register_bm_callback, &DEFAULT_BEATMAP);
}
