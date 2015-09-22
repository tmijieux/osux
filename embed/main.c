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


int main(int argc, char *argv[])
{
    MonoDomain *domain;
    /* MonoAssembly *assembly; */
    
    domain = mono_jit_init( "date.exe" );
    /* assembly = mono_domain_assembly_open(domain, "date.exe"); */
    /* mono_jit_exec(domain, assembly, argc-1, argv+1); */
    
    return 0;
}
