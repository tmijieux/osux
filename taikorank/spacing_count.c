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
#include "util/list.h"

static void sp_if_increase_f(struct spacing * sp, int ** data);
static void sp_if_increase(struct spacing * sp, int * rest);
static void sp_print(struct spacing * sp);

struct list * spc_new(void)
{
    return list_new(0);
}

void spc_free(struct list * spc)
{
    list_each(spc, free);
    list_free(spc);
}

void spc_add(struct list * spc, int rest)
{
    struct spacing * sp = malloc(sizeof(*sp));
    sp->rest = rest;
    sp->nb = 1;
    list_append(spc, sp);
}

void spc_add_f(struct list * spc, int rest, double val)
{
    struct spacing * sp = malloc(sizeof(*sp));
    sp->rest = rest;
    sp->nb = val;
    list_append(spc, sp);
}

static void sp_if_increase(struct spacing * sp, int * rest)
{
    if(sp->rest == *rest)
	sp->nb++;
}

void spc_increase(struct list * spc, int rest)
{
    int value = rest;
    list_each_r(spc, (void (*)(void *, void *))sp_if_increase, &value);
}

static void sp_if_increase_f(struct spacing * sp, int ** data)
{
    int rest = *(data[0]);
    double i = *((double *) (data[1]));
    if(sp->rest == rest)
	sp->nb += i;
}

void spc_increase_f(struct list * spc, int rest, double val)
{
    int rest_2 = rest;
    double i = val;
    void * data[2];
    data[0] = &rest_2;
    data[1] = &i;
    list_each_r(spc, (void (*)(void *, void *))sp_if_increase_f, data);
}

static void sp_print(struct spacing * sp)
{
    printf("space: %d \t (%g)\n", sp->rest, sp->nb);
}

void spc_print(struct list * spc)
{
    printf("spacing\n");
    list_each(spc, (void (*)(void *))sp_print);
}

double spc_get_total(struct list * spc)
{
    double res = 0;
    unsigned int len = list_size(spc);
    for(unsigned int i = 1; i <= len; i++) {
	struct spacing * sp = list_get(spc, i);
	res += sp->nb;
    }
    return res;
}

double spc_get_nb(struct list * spc, int rest, int (*eq)(int, int))
{
    unsigned int len = list_size(spc);
    for(unsigned int i = 1; i <= len; i++) {
	struct spacing * sp = list_get(spc, i);
	if(eq(sp->rest, rest))
	    return sp->nb;
    }
    return 0;
}
