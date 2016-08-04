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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "osux/hash_table.h"
#include "osux/list.h"

#define INITIAL_HASH_TABLE_SIZE     113

struct osux_hashtable_entry {
    char *key;
    void *data;
    struct osux_hashtable_entry *next;
};

struct osux_hashtable {
    size_t size;
    struct osux_hashtable_entry **buf;
    int entry_count;

    int (*hash) (const char*);
    void (*free)(void*);
};

static const unsigned char T[256] = {
    // 256 values 0-255 in any (random) order suffices
    98,  6, 85,150, 36, 23,112,164,135,207,169,  5, 26, 64,165,219, //  1
    61, 20, 68, 89,130, 63, 52,102, 24,229,132,245, 80,216,195,115, //  2
    90,168,156,203,177,120,  2,190,188,  7,100,185,174,243,162, 10, //  3
    237, 18,253,225,  8,208,172,244,255,126,101, 79,145,235,228,121, //  4
    123,251, 67,250,161,  0,107, 97,241,111,181, 82,249, 33, 69, 55, //  5
    59,153, 29,  9,213,167, 84, 93, 30, 46, 94, 75,151,114, 73,222, //  6
    197, 96,210, 45, 16,227,248,202, 51,152,252,125, 81,206,215,186, //  7
    39,158,178,187,131,136,  1, 49, 50, 17,141, 91, 47,129, 60, 99, //  8
    154, 35, 86,171,105, 34, 38,200,147, 58, 77,118,173,246, 76,254, //  9
    133,232,196,144,198,124, 53,  4,108, 74,223,234,134,230,157,139, // 10
    189,205,199,128,176, 19,211,236,127,192,231, 70,233, 88,146, 44, // 11
    183,201, 22, 83, 13,214,116,109,159, 32, 95,226,140,220, 57, 12, // 12
    221, 31,209,182,143, 92,149,184,148, 62,113, 65, 37, 27,106,166, // 13
    3, 14,204, 72, 21, 41, 56, 66, 28,193, 40,217, 25, 54,179,117, // 14
    238, 87,240,155,180,170,242,212,191,163, 78,218,137,194,175,110, // 15
    43,119,224, 71,122,142, 42,160,104, 48,247,103, 15, 11,138,239  // 16
};


static int default_hash(const char *x)
{
 // Pearson's hash:
 // adapted from :
 // http://cs.mwsu.edu/~griffin/courses/2133/downloads/Spring11/p677-pearson.pdf

    union {
        unsigned char hh[4];
        uint32_t i;
    } v;
    for (int j = 0; j < 4; j++) {
	unsigned char h = T[(x[0] + j) % 256];
	for (int i = 0; x[i]; i++) {
	    h = T[h ^ x[i]];
	}
        v.hh[j] = h;
    }

    return (int) v.i;
}

static struct osux_hashtable_entry* new_entry(const char *key, void *data)
{
    struct osux_hashtable_entry *he = malloc(sizeof(*he));
    he->key = strdup(key);
    he->data = data;
    he->next = NULL;
    return he;
}

static void free_entry(struct osux_hashtable_entry *he)
{
}

osux_hashtable* osux_hashtable_new(size_t size)
{
    return osux_hashtable_new_full(size, NULL);
}

osux_hashtable* osux_hashtable_new_full(size_t size, void (*free_v)(void*))
{
    osux_hashtable *ht = (osux_hashtable*) malloc(sizeof(*ht));
    ht->hash = &default_hash;
    if (size > 0)
	    ht->size = size;
    else
	    ht->size = INITIAL_HASH_TABLE_SIZE;
    ht->free = free_v;

    ht->buf = calloc(sizeof(*ht->buf), ht->size);
    return ht;
}

int osux_hashtable_insert(osux_hashtable* ht, const char *key, void *data)
{
    int hash = ht->hash(key);
    int pos = hash % ht->size;

    struct osux_hashtable_entry *he = new_entry(key, data);
    he->next = ht->buf[pos];
    ht->buf[pos] = he;

    return 0;
}

int osux_hashtable_remove(osux_hashtable *ht, const char *key)
{
    int hash = ht->hash(key);
    int pos = hash % ht->size;

    struct osux_hashtable_entry *he = ht->buf[pos], *prev = NULL;

    while (he) {
	if (!strcmp(he->key, key)) {
	    if (prev)
		prev->next = he->next;
	    else
		ht->buf[pos] = he->next;

            free(he->key);
            free(he);
	}
	prev = he;
	he = he->next;
    }
    return -1;
}

int osux_hashtable_contains(osux_hashtable *ht, const char *key)
{
    int hash = ht->hash(key);
    int pos = hash % ht->size;

    struct osux_hashtable_entry *he = ht->buf[pos];

    while (he) {
	if (!strcmp(he->key, key))
	    return 1;
	he = he->next;
    }
    return 0;
}

int osux_hashtable_lookup(osux_hashtable *ht, const char *key, void *ret)
{
    int hash = ht->hash(key);
    int pos = hash % ht->size;

    struct osux_hashtable_entry *he = ht->buf[pos];

    while (he) {
	if (!strcmp(he->key, key)) {
	    *(const void**) ret = he->data;
	    return 0;
	}
	he = he->next;
    }
	*(const void**) ret = NULL;
    return -1;
}

int osux_hashtable_hash(osux_hashtable *ht, const char *key)
{
    return ht->hash(key);
}

void osux_hashtable_delete(osux_hashtable* ht)
{
    if (ht) {
	for (unsigned i = 0; i < ht->size; ++i) {
	    struct osux_hashtable_entry *he = ht->buf[i];
	    while (he) {
		struct osux_hashtable_entry *tmp = he;
		he = he->next;
                free(tmp->key);

                if (ht->free)
                    ht->free(tmp->data);
                free(tmp);
	    }
	}
	free(ht->buf);
	free(ht);
    }
}

void osux_hashtable_for_each(osux_hashtable* ht,
		 void (*fun)(const char *, void*, void*), void *args)
{
    if (ht) {
	for (unsigned i = 0; i < ht->size; ++i) {
	    struct osux_hashtable_entry *he = ht->buf[i];
	    while (he) {
		fun(he->key, he->data, args);
		he = he->next;
	    }
	}
    }
}


struct osux_list* osux_hashtable_to_list(const osux_hashtable *ht)
{
    struct osux_list *l = osux_list_new(0);
    if (ht) {
	for (unsigned i = 0; i < ht->size; ++i) {
	    struct osux_hashtable_entry *he = ht->buf[i];
	    while (he) {
		osux_list_append(l, he->data);
		he = he->next;
	    }
	}
    }
    return l;
}
