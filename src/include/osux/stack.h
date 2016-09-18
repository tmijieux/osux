#ifndef OSUX_STACK_H
#define OSUX_STACK_H

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
#include <glib.h>

G_BEGIN_DECLS

#ifndef STACK_TYPE
#define STACK_TYPE void*
#define STACK_TYPE_INITIALIZER NULL
#endif


struct stack;

struct stack *stack_create(size_t buf_size);
void stack_destroy(struct stack *stack);

void stack_push(struct stack *s, STACK_TYPE element);
STACK_TYPE stack_pop(struct stack *s);
STACK_TYPE stack_peek(const struct stack *s);
int stack_is_empty(const struct stack *s);
int stack_is_full(const struct stack *s);
size_t stack_size(const struct stack *s);

G_END_DECLS

#endif // OSUX_STACK_H
