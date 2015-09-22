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

#include "list.h"
#include "list_node.h"

struct list {
    struct list_node *front_sentinel;
    struct list_node *cursor;
    size_t size;
    unsigned int curpos;
    unsigned int flags;
};

static struct list_node *list_get_node(struct list *list, unsigned int n)
{
	int k = n;
	struct list_node *node = list->front_sentinel;
	if (list->curpos <= k) {
		k -= list->curpos;
		node = list->cursor;
	}
	for (int i = 0; i < k; i++)
	    node = node_get_next(node);
	list->cursor = node;
	list->curpos = n;
	return node;
}

size_t list_size(struct list *list)
{
	return list->size;
}

struct list *list_new(int flags)
{
	struct list *list = malloc(sizeof(*list));
	list->front_sentinel = node_new(NULL, SENTINEL_NODE);
	node_set_next(list->front_sentinel, node_new(NULL, 1));
	list->flags = flags;
	list->cursor = list->front_sentinel;
	list->curpos = 0;
	list->size = 0;
	return list;
}

void list_free(struct list *list)
{
	struct list_node *node = node_get_next(list->front_sentinel);
	struct list_node *tmp = NULL;
	while (!node_is_sentinel(node)) {
		tmp = node_get_next(node);
		if ((list->flags & LI_FREEMALLOCD) != 0)
			free(node_get_data(node));
		node_free(node);
		node = tmp;
	}
	node_free(node); //the back sentinel
	node_free(list->front_sentinel);
	free(list);
}

void *list_get(struct list *list, unsigned int n)
{
	return node_get_data(list_get_node(list, n));
}

void list_add(struct list *list, void *element)
{
	struct list_node *tmp = node_new(element, 0);
	node_set_next(tmp, node_get_next(list->front_sentinel));
	node_set_next(list->front_sentinel, tmp);
	list->size ++;
}

void list_insert(struct list *list, unsigned int n, void *element)
{
	struct list_node *previous = list_get_node(list, n-1);
	struct list_node *tmp = node_new(element, 0);
	node_set_next(tmp, node_get_next(previous));
	node_set_next(previous, tmp);
	list->size ++;
}

void list_remove(struct list *list, unsigned int n)
{
	struct list_node *previous = list_get_node(list, n-1);
	struct list_node *tmp = node_get_next(previous);
	node_set_next(previous, node_get_next(tmp));
	if ((list->flags & LI_FREEMALLOCD) != 0)
		free(node_get_data(tmp));
	node_free(tmp);
	list->size --;
}


