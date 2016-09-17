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

struct spacing_count {
    osux_list * l;
    int (*eq)(int, int);
};

struct spacing {
    int rest;
    double nb;
};

static void sp_print(const struct spacing * sp);

//--------------------------------------------------

struct spacing_count * spc_new(int (*eq)(int, int))
{
    struct spacing_count * spc = malloc(sizeof(*spc));
    spc->l = osux_list_new(0);
    spc->eq = eq;
    return spc;
}

void spc_free(struct spacing_count * spc)
{
    if (spc == NULL)
        return;
    osux_list_each(spc->l, free);
    osux_list_free(spc->l);
    free(spc);
}

void spc_add(struct spacing_count * spc, int rest, double val)
{
    unsigned int len = osux_list_size(spc->l);
    for (unsigned int i = 1; i <= len; i++) {
        struct spacing * sp = osux_list_get(spc->l, i);
        if (spc->eq(sp->rest, rest)) {
            sp->nb += val;
            return;
        }
    }

    struct spacing * sp = malloc(sizeof(*sp));
    sp->rest = rest;
    sp->nb = val;
    osux_list_append(spc->l, sp);
}

static void sp_print(const struct spacing * sp)
{
    printf("space: %d \t (%g)\n", sp->rest, sp->nb);
}

void spc_print(const struct spacing_count * spc)
{
    printf("spacing\n");
    osux_list_each(spc->l, (void (*)(void *))sp_print);
}

double spc_get_total(const struct spacing_count * spc)
{
    double res = 0;
    unsigned int len = osux_list_size(spc->l);
    for (unsigned int i = 1; i <= len; i++) {
        struct spacing * sp = osux_list_get(spc->l, i);
        res += sp->nb;
    }
    return res;
}

double spc_get_nb(const struct spacing_count * spc, int rest)
{
    unsigned int len = osux_list_size(spc->l);
    for (unsigned int i = 1; i <= len; i++) {
        struct spacing * sp = osux_list_get(spc->l, i);
        if (spc->eq(sp->rest, rest))
            return sp->nb;
    }
    return 0;
}
