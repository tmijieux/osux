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

#include "osux/hash_table.h"
#include "osux/list.h"
#include "./uthash.h"

struct osux_hashtable_entry {
    char *key;
    void *data;
    UT_hash_handle hh;
};

typedef struct osux_hashtable_ {
    struct osux_hashtable_entry *h;
    void (*free)(void*);
} osux_hashtable;

int osux_hashtable_entry_count(osux_hashtable const *h)
{
    return HASH_COUNT(h->h);
}

osux_hashtable *osux_hashtable_new(size_t size)
{
    return osux_hashtable_new_full(size, NULL);
}

osux_hashtable *osux_hashtable_new_full(size_t size, void(*free_)(void*))
{
    (void) size;
    osux_hashtable *h = calloc(sizeof*h, 1);
    h->h = NULL;
    h->free = free_;
    return h;
}

int osux_hashtable_insert(osux_hashtable *h, const char *key, void *data)
{
    struct osux_hashtable_entry *entry = malloc(sizeof*entry);
    entry->data = (void*) data;
    entry->key = strdup(key);

    HASH_ADD_STR(h->h, key, entry);
    return 0;
}

int osux_hashtable_add_unique_entry(osux_hashtable *h, const char *key, void *data)
{
    int err = -1;
    struct osux_hashtable_entry *entry;

    HASH_FIND_STR(h->h, key, entry);
    if (entry == NULL) {
        entry = malloc(sizeof*entry);
        entry->data = (void*) data;
        entry->key = strdup(key);
        HASH_ADD_STR(h->h, key, entry);
        err = 0;
    }
    return err;
}

int osux_hashtable_remove(osux_hashtable *h, const char *key)
{
    int err = -1;
    struct osux_hashtable_entry *entry;

    HASH_FIND_STR(h->h, key, entry);
    if (entry != NULL) {
        HASH_DEL(h->h, entry);
        free(entry->key);
        free(entry);
        err = 0;
    }
    return err;
}

int osux_hashtable_contains(osux_hashtable *h, const char *key)
{
    int res = 0;
    struct osux_hashtable_entry *entry;

    HASH_FIND_STR(h->h, key, entry);
    res = (entry != NULL);
    return res;
}

int osux_hashtable_lookup(osux_hashtable *h, const char *key, void *ret)
{
    int err = -1;
    struct osux_hashtable_entry *entry;

    HASH_FIND_STR(h->h, key, entry);
    if (NULL != entry) {
        *((void**)ret) = entry->data;
        err = 0;
    }
    return err;
}

void osux_hashtable_delete(osux_hashtable *h)
{
    struct osux_hashtable_entry *entry, *tmp;
    HASH_ITER(hh, h->h, entry, tmp) {
        HASH_DEL(h->h, entry);
        if (h->free) h->free(entry->data);
        free(entry->key);
        free(entry);
    }
    free(h);
}

void osux_hashtable_each_r(osux_hashtable *h,
		 void (*fun)(const char *, void*, void*), void *args)
{
    struct osux_hashtable_entry *entry, *tmp;

    HASH_ITER(hh, h->h, entry, tmp) {
        fun(entry->key, entry->data, args);
    }
}

void osux_hashtable_each(osux_hashtable *h,
		 void (*fun)(const char *, void*))
{
    struct osux_hashtable_entry *entry, *tmp;

    HASH_ITER(hh, h->h, entry, tmp) {
        fun(entry->key, entry->data);
    }
}

osux_list* osux_hashtable_to_list(osux_hashtable *h)
{
    struct osux_hashtable_entry *entry, *tmp;
    osux_list *l = osux_list_new(0);

    HASH_ITER(hh, h->h, entry, tmp) {
        osux_list_add(l, entry->data);
    }
    return l;
}
