#ifndef OSUX_LIST_H
#define OSUX_LIST_H

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
#include <assert.h>
#include <glib.h>

typedef struct osux_list_ osux_list;

#include "osux/hash_table.h"

G_BEGIN_DECLS

// FLAGS for list_new():
#define LI_ELEM  (1 << 1)	// null terminated initializer list
#define LI_FREE  (1 << 2)	// second argument is a free function for elements
// END FLAGS



// ! !
// NUMBERED FROM 1 !!!!!!!!
// ! !

#define list_addg(typ, li, da)						\
    ({ assert(sizeof typ < sizeof (void*)) ;				\
	union { typ t; void *v}e; e.t = da; list_push(li, e.v);})

#define list_getg(typ, li, i)						\
    ({ assert(sizeof typ < sizeof (void*)) ;				\
	union { typ t; void *v}e; e.v = list_get((li), (i)); e.t;})

osux_list *osux_list_new(int flags, ...);
void osux_list_free(osux_list*);
unsigned osux_list_size(osux_list const*); // element count
void *osux_list_get(osux_list const*, unsigned int i); // returns data
void osux_list_add(osux_list*, const void*);
void osux_list_insert(osux_list*, unsigned int i, const void *data);
void osux_list_remove(osux_list*, unsigned int i);

void osux_list_append(osux_list *list, const void *element);
void osux_list_append_list(osux_list *l1, osux_list const *l2);
osux_list *osux_list_copy(osux_list const *l);
void *osux_list_to_array(osux_list const *l);
osux_hashtable *osux_list_to_hashtable(
    osux_list const *l, const char *(*element_keyname) (void *));
void osux_list_each(osux_list const *l, void (*fun)(void*));
void osux_list_each_r(
    osux_list const *l, void (*fun)(void*, void*), void *args);
osux_list *osux_list_map(osux_list const *l, void *(*fun)(void*));
osux_list *osux_list_map_r(
    osux_list const *l, void *(*fun)(void*, void*), void *args);


G_END_DECLS

#endif // OSUX_LIST_H
