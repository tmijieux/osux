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

#ifndef LIST_NODE_H
#define LIST_NODE_H

#include <stdlib.h>

struct list_node {
    const void *data;
    struct list_node *next;
    int is_sentinel;
};

#define SENTINEL_NODE 1
#define DATA_NODE     0

#define node_new(da, sen)				\
    ({							\
	struct list_node *n = malloc(sizeof(*n));	\
	n->data = (da);					\
	n->is_sentinel = (sen);				\
	n;						\
    })							\
    
#define node_free(no)	       (free(no))
#define node_get_next(no)      ((no)->next)
#define node_set_next(no, ne)  ((no)->next = (ne))
#define node_get_data(no)      ((no)->data)
#define node_set_data(no, da)  ((no)->data = (da))
#define node_is_sentinel(no)   ((no)->is_sentinel)

#endif //LIST_NODE_H
