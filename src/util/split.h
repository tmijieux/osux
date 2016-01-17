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

#ifndef SPLIT_H
#define SPLIT_H

unsigned int string_split(const char *str, const char *delim, char ***buf_addr);
int string_have_extension(const char *filename, const char *extension);
void read_string_ULEB128(char **buf, FILE *f);
void write_string_ULEB128(char *buf, FILE *f);

#endif //SPLIT_H
