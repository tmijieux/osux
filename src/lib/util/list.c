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
n */

#include <stdlib.h>
#include <stdarg.h>

#include "osux/list.h"
#include "osux/list_node.h"
#include "osux/hash_table.h"

struct osux_list {
    struct osux_list_node *front_sentinel;
    struct osux_list_node *last;
    void (*free_element)(void*);
    struct osux_list_node *cursor;
    unsigned size;
    unsigned int curpos;
    unsigned int flags;
};

static struct osux_list_node *
osux_list_get_node(const struct osux_list *list, unsigned int n)
{
    unsigned int k = n;
    struct osux_list_node *node = list->front_sentinel;
    if (list->curpos <= k) {
	k -= list->curpos;
	node = list->cursor;
    }
    for (unsigned int i = 0; i < k; i++)
	node = node_get_next(node);
    ((struct osux_list*)list)->cursor = node;
    ((struct osux_list*)list)->curpos = n;
    return node;
}

unsigned
osux_list_size(const struct osux_list *list)
{
    return list->size;
}

struct osux_list *
osux_list_new(int flags, ...)
{
    struct osux_list_node *tmp;
    struct osux_list *list = calloc(sizeof(*list), 1);
    
    node_new(&list->front_sentinel, NULL, SENTINEL_NODE);
    node_new(&tmp, NULL, 1);
    node_set_next(list->front_sentinel, tmp);
    
    list->flags = flags;
    list->cursor = list->front_sentinel;
    list->curpos = 0;
    list->size = 0;

    va_list ap;
    va_start(ap, flags);
    if ((flags & LI_FREE) != 0) {
        typedef void (*fun_ptr_t)(void*);
        list->free_element = va_arg(ap, fun_ptr_t);
    }
    if ((flags & LI_ELEM) != 0) {
        void *arg;
        do {
            arg = va_arg(ap, void*);
            if (arg != NULL)
                osux_list_append(list, arg);
        } while (NULL != arg);
    }

    return list;
}

void
osux_list_free(struct osux_list *list)
{
    struct osux_list_node *node = node_get_next(list->front_sentinel);
    struct osux_list_node *tmp = NULL;
    while (!node_is_sentinel(node)) {
	tmp = node_get_next(node);
	if ((list->flags & LI_FREE) != 0)
	    list->free_element((void*) node_get_data(node));
	node_free(node);
	node = tmp;
    }
    node_free(node); //the back sentinel
    node_free(list->front_sentinel);
    free(list);
}

void *
osux_list_get(const struct osux_list *list, unsigned n)
{
    return (void*) node_get_data(osux_list_get_node(list, n));
}

void
osux_list_add(struct osux_list *list, const void *element)
{
    struct osux_list_node *tmp;
    node_new(&tmp, element, 0);
    node_set_next(tmp, node_get_next(list->front_sentinel));
    node_set_next(list->front_sentinel, tmp);
    if (list->curpos != 0)
        list->curpos++;
    list->size ++;
}

void
osux_list_append(struct osux_list *list, const void *element)
{
    int n = osux_list_size(list) + 1;
    osux_list_insert(list, n, element);
}

void
osux_list_append_list(struct osux_list *l1, const struct osux_list *l2)
{
    for (unsigned int i = 1; i <= osux_list_size(l2); ++i)
	osux_list_append(l1, osux_list_get(l2, i));
}

void
osux_list_insert(struct osux_list *list, unsigned int n, const void *element)
{
    struct osux_list_node *previous = osux_list_get_node(list, n-1);
    struct osux_list_node *tmp;
    node_new(&tmp, element, 0);
    node_set_next(tmp, node_get_next(previous));
    node_set_next(previous, tmp);
    list->size ++;
}

void
osux_list_remove(struct osux_list *list, unsigned int n)
{
    struct osux_list_node *previous = osux_list_get_node(list, n-1);
    struct osux_list_node *tmp = node_get_next(previous);
    node_set_next(previous, node_get_next(tmp));
    if ((list->flags & LI_FREE) != 0)
	list->free_element((void*)node_get_data(tmp));
    node_free(tmp);
    list->size --;
}

struct osux_list *
osux_list_copy(const struct osux_list *l)
{
    struct osux_list *n = osux_list_new(0);
    osux_list_append_list(n, l);
    return n;
}

void *osux_list_to_array(const struct osux_list *l)
{
    void **array = malloc(sizeof(*array) * l->size);
    for (unsigned i = 0; i < l->size; ++i)
        array[i] = osux_list_get(l, i+1);

    return array;
}

struct hash_table *
osux_list_to_hashtable(const struct osux_list *l,
                       const char *(*element_keyname) (void *))
{
    struct hash_table *ht = ht_create(2 * l->size, NULL);

    for (unsigned i = 0; i < l->size; ++i) {
	void *el = osux_list_get(l, i+1);
	ht_add_entry(ht, element_keyname(el), el);
    }
    return ht;
}

static void *
uncurry(void *arg, void *fun)
{
    return ((void *(*)(void*)) fun)(arg);
}

struct osux_list *
osux_list_map(const struct osux_list *l, void *(*fun)(void*))
{
    return osux_list_map_r(l, &uncurry, fun);
}

void
osux_list_each(const struct osux_list *l, void (*fun)(void*))
{
    osux_list_each_r(l, (void (*)(void*,void*))&uncurry, fun);
}

struct osux_list *
osux_list_map_r(const struct osux_list *l,
                void *(*fun)(void*, void*), void *args)
{
    int si = osux_list_size(l);
    struct osux_list *ret= osux_list_new(0);
    for (int i = 1; i <= si; ++i)
	osux_list_append(ret, fun(osux_list_get(l, i), args));
    return ret;
}

void
osux_list_each_r(const struct osux_list *l, void (*fun)(void*, void*), void *args)
{
    int si = osux_list_size(l);
    for (int i = 1; i <= si; ++i)
	fun(osux_list_get(l, i), args);
}
