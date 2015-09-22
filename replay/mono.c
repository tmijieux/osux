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


#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

int cs_timestamp_string(uint64_t timestamp)
{
	char buf[50] = {0};
	sprintf(buf, "%lu", timestamp);
	int argc = 1;
	char *argv[] = { buf };

	MonoDomain *domain = NULL;
	domain = mono_jit_init( "date.exe" );
	
	MonoAssembly *assembly;
	assembly = mono_domain_assembly_open(domain, "date.exe");

    //return mono_jit_exec(domain, assembly, argc, argv);
	return 0;
}
