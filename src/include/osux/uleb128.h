#ifndef OSUX_ULEB128_H
#define OSUX_ULEB128_H

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

#include <stdio.h>
#include <stdint.h>
#include <glib.h>

G_BEGIN_DECLS

uint64_t read_ULEB128(FILE * f);
void write_ULEB128(uint64_t value, FILE *output, unsigned padding);

G_END_DECLS

#endif //ULEB128_H
