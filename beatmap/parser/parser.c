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

#ifdef __linux__
#	include <dlfcn.h>
#elif _WIN32
#	include <windows.h>
#endif

#include "beatmap/beatmap.h"
#include "util/data.h"
#include "util/error.h"
#include "initializer.h"
#include "./parser.h"

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
    #ifdef __linux__
    void *handle;
    handle = dlopen(PKG_LIB_DIR"/libosux_parser_py.so", RTLD_LOCAL|RTLD_NOW);
    if (NULL == handle) {
        osux_error("file: %s\n", PKG_LIB_DIR"/libosux_parser_py.so");
        osux_error("Failed to initialize parser:\n%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    parser_py_init = dlsym(handle, "parser_py_init");
    
    #elif _WIN32
    HINSTANCE hinstLib = LoadLibrary(PKG_LIB_DIR"/libosux_parser_py.dll");
    if (hinstLib == NULL)
    {
        MessageBox(NULL, "Unable to load library", "Error", MB_OK|MB_ICONERROR);
        return 0;
    }
    parser_py_init = (plugin_init_t)
        GetProcAddress( hinstLib, "parser_py_init" );
    #endif
    
    if (parser_py_init != NULL) {
        parser_py_init(&osux_register_bm_callback);
    } else {
        #ifdef __linux__
        osux_error("WTF: %s\n", dlerror());
        #endif
    }
}
