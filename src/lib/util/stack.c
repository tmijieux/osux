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

#include "osux/stack.h"

struct osux_stack_ {
    GList *l;
    unsigned size;
};

unsigned
osux_stack_size(osux_stack const *s)
{
    return s->size;
}

osux_stack *osux_stack_new(void)
{
    osux_stack *s = g_malloc0(sizeof*s);
    return s;
}

void
osux_stack_push(osux_stack *s, gpointer data)
{
    ++ s->size;
    s->l = g_list_prepend(s->l, data);
}

gpointer
osux_stack_head(osux_stack const *s)
{
    g_assert(s->l != NULL);
    return s->l->data;
}

gpointer
osux_stack_pop(osux_stack *s)
{
    g_assert(s->l != NULL);
    -- s->size;
    void *data = s->l->data;
    s->l = g_list_delete_link(s->l, s->l);
    return data;
}

bool
osux_stack_is_empty(osux_stack const *s)
{
    return s->size == 0;
}

void
osux_stack_delete(osux_stack *s)
{
    g_list_free(s->l);
    s->l = (gpointer) 0xdeadbeefcafebabe; // garbage memory
    s->size = 0xDEADBEEF;
    g_free(s);
}
