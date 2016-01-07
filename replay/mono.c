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

#include <stdlib.h>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

#include "mono.h"

int cs_timestamp_string(uint64_t timestamp)
{
    MonoDomain *domain = NULL;
    domain = mono_jit_init("date.dll");
	
    MonoAssembly *assembly;
    assembly = mono_domain_assembly_open(domain, "date.dll");
    if (!assembly) {
	fprintf(stderr, "error loading assembly file date.dll");
	return -1;
    }
    MonoImage * image = mono_assembly_get_image(assembly);
    MonoMethodDesc* get_date_desc =
	mono_method_desc_new("OsuDate:get_date(long)", 1);
    MonoMethod *get_date_method =
    	mono_method_desc_search_in_image(get_date_desc, image);
    void *params[1] = { &timestamp };
    MonoObject * ret = mono_runtime_invoke(get_date_method, NULL, params, NULL);
    char *str = mono_string_to_utf8((MonoString*)ret);
    fputs(str, stdout);
    return 0;
}
