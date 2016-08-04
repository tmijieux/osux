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

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdlib.h>

typedef struct osux_hashtable osux_hashtable;

struct osux_hashtable* osux_hashtable_new(size_t size);
struct osux_hashtable* osux_hashtable_new_full(size_t size, void(*free)(void*));

int osux_hashtable_insert(osux_hashtable* ht, const char *key, void *data);
int osux_hashtable_remove(osux_hashtable *ht, const char *key);
int osux_hashtable_contains(osux_hashtable *ht, const char *key);
int osux_hashtable_lookup(osux_hashtable *ht,
                          const char *lookup_key,
                          void *ret_value);
int osux_hashtable_hashstr(osux_hashtable *ht, const char *key);
void osux_hashtable_for_each(
    osux_hashtable* ht, void (*fun)(const char *, void*, void*), void *args);
void osux_hashtable_delete(osux_hashtable *ht);

#endif //HASH_TABLE_H
