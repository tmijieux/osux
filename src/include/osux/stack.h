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

#ifndef OSUX_STACK_H
#define OSUX_STACK_H

#include <glib.h>
#include <stdbool.h>

G_BEGIN_DECLS

typedef struct osux_stack_ osux_stack;

osux_stack *osux_stack_new(void);
unsigned osux_stack_size(osux_stack const *s);
void osux_stack_push(osux_stack *s, void *data);
gpointer osux_stack_head(osux_stack const *s);
gpointer osux_stack_pop(osux_stack *s);
bool osux_stack_is_empty(osux_stack const *s);
void osux_stack_delete(osux_stack *s);

G_END_DECLS

#endif // OSUX_STACK_H
