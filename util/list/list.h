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

#ifndef LIST_H
#define LIST_H

// FLAGS for list_new():
#define LI_FREEMALLOCD (1 << 0)  // attempt to free all its elements when
                                 // list_free() is called
// END FLAGS

struct list;

// ! !
// NUMBERED FROM 1 !!!!!!!!
// ! !

struct list *list_new(int flags);
void list_free(struct list*);
size_t list_size(struct list*); // element count
void *list_get(struct list*, unsigned int i); // returns data
void list_add(struct list*, void*);
void list_insert(struct list*, unsigned int i, void *data);
void list_append(struct list*, void *data);
void list_remove(struct list*, unsigned int i);

#endif //LIST_H
