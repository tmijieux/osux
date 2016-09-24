/*
 *  Copyright (©) 2015-2016 Lucas Maugère, Thomas Mijieux
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

#include "spacing_count.h"
#include "osux.h"

/*
  Most of the time the list is short: 3 or 4 elements.
  Using a sorted list might increase a little performances.
 */
struct spacing_count {
    GList *l;
    int (*eq)(int, int);
};

struct spacing {
    int rest;
    double nb;
};

static void sp_print(const struct spacing *sp);

//--------------------------------------------------

struct spacing_count *spc_new(int (*eq)(int, int))
{
    struct spacing_count *spc = g_malloc(sizeof*spc);
    spc->l = NULL;
    spc->eq = eq;
    return spc;
}

void spc_free(struct spacing_count *spc)
{
    if (spc == NULL)
        return;
    g_list_free_full(spc->l, g_free);
    g_free(spc);
}

void spc_add(struct spacing_count *spc, int rest, double val)
{
    GList *it;
    for (it = spc->l; it != NULL; it = it->next) {
        struct spacing *sp = it->data;
        if (spc->eq(sp->rest, rest)) {
            sp->nb += val;
            return;
        }
    }

    struct spacing *sp = g_malloc(sizeof(*sp));
    sp->rest = rest;
    sp->nb = val;
    spc->l = g_list_prepend(spc->l, sp);
}

static void sp_print(const struct spacing *sp)
{
    printf("space: %d \t (%g)\n", sp->rest, sp->nb);
}

void spc_print(const struct spacing_count *spc)
{
    printf("spacing\n");
    g_list_foreach(spc->l, (GFunc) sp_print, NULL);
}

double spc_get_total(const struct spacing_count *spc)
{
    double res = 0;
    GList *it;
    for (it = spc->l; it != NULL; it = it->next) {
        struct spacing *sp = it->data;
        res += sp->nb;
    }
    return res;
}

double spc_get_nb(const struct spacing_count *spc, int rest)
{
    GList *it;
    for (it = spc->l; it != NULL; it = it->next) {
        struct spacing *sp = it->data;
        if (spc->eq(sp->rest, rest))
            return sp->nb;
    }
    return 0;
}
