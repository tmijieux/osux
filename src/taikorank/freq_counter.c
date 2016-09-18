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
#include <math.h>

#include "osux.h"
#include "freq_counter.h"

typedef void (*ht_fun)(const char*,void*,void*);

struct counter {
    osux_hashtable *ht;
    double total;
};

struct counter_entry {
    const void *data;
    double nb;
};

struct inheriter {
    const struct counter *c;
    struct counter_entry *e;
    union {
        inherit_fun inherit;
        where_fun where;
    } fun;
    double nb;
};

static struct counter_entry *cnte_new(const void *d, double nb);
static void cnte_free(const char *key, struct counter_entry *e);
static void cnte_print(
    const char *key, struct counter_entry *e, struct inheriter *h);
static void cnte_inherit(
    const char *key, struct counter_entry *e, struct inheriter *h);
static void cnte_add_nb_inherit(
    const char *key, struct counter_entry *e, struct inheriter *h);

//--------------------------------------------------

static struct counter_entry *cnte_new(const void *d, double nb)
{
    struct counter_entry *e = malloc(sizeof(*e));
    e->data = d;
    e->nb = nb;
    return e;
}

static void cnte_free(const char *key UNUSED, struct counter_entry *e)
{
    if (e == NULL)
        return;
    free(e);
}

//--------------------------------------------------

static void cnte_print(const char *key,
                       struct counter_entry *e,
                       struct inheriter *h)
{
    if (h == NULL) {
        printf("Entry:\t%s\t%.4g\t%.4f\n",
               key, e->nb, e->nb / h->c->total);
    } else {
        double d = cnt_get_nb_inherit(h->c, key, h->fun.inherit);
        printf("Entry:\t%s\t%.4g\t%.4g\t%.4f\t%.4f\n",
               key, e->nb, d, e->nb / h->c->total, d / h->c->total);
    }
}

//--------------------------------------------------

static void cnte_inherit(const char *key UNUSED,
                         struct counter_entry *e,
                         struct inheriter *h)
{
    h->nb += e->nb * h->fun.inherit(h->e->data, e->data);
}

static void cnte_add_nb_inherit(const char *key,
                                struct counter_entry *e UNUSED,
                                struct inheriter *h)
{
    h->nb += cnt_get_nb_inherit(h->c, key, h->fun.inherit);
}

static void cnte_where(const char *key UNUSED,
                       struct counter_entry *e,
                       struct inheriter *h)
{
    h->nb += e->nb * h->fun.where(e->data);
}

//--------------------------------------------------

struct counter *cnt_new(void)
{
    struct counter *c = malloc(sizeof(*c));
    c->ht = osux_hashtable_new(0);
    c->total = 0;
    return c;
}

void cnt_free(struct counter *c)
{
    if (c == NULL)
        return;
    osux_hashtable_each(c->ht,
                        (void (*)(const char*, void*)) cnte_free);
    osux_hashtable_delete(c->ht);
    free(c);
}

//--------------------------------------------------

void cnt_add(struct counter *c, const void *data,
             const char *key, double val)
{
    struct counter_entry *e = NULL;
    osux_hashtable_lookup(c->ht, key, &e);
    if (e != NULL) {
        e->nb += val;
    } else {
        e = cnte_new(data, val);
        osux_hashtable_insert(c->ht, key, e);
    }
    c->total += val;
}

//--------------------------------------------------

double cnt_get_nb_inherit(const struct counter *c,
                          const char *key, inherit_fun inherit)
{
    struct counter_entry *e = NULL;
    osux_hashtable_lookup(c->ht, key, &e);
    if (e == NULL)
        return 0;
    struct inheriter total = {NULL, e, {inherit}, 0};
    osux_hashtable_each_r(c->ht, (ht_fun) cnte_inherit, &total);
    return total.nb;
}

double cnt_get_nb(const struct counter *c, const char *key)
{
    struct counter_entry *e = NULL;
    osux_hashtable_lookup(c->ht, key, &e);
    if (e == NULL)
        return 0;
    return e->nb;
}

double cnt_get_nb_where(const struct counter *c, where_fun where)
{
    struct inheriter h = {c, NULL, {.where = where}, 0};
    osux_hashtable_each_r(c->ht, (ht_fun) cnte_where, &h);
    return h.nb;
}

double cnt_get_total(const struct counter *c)
{
    return c->total;
}

double cnt_get_total_inherit(const struct counter *c, inherit_fun inherit)
{
    struct inheriter h = {c, NULL, {inherit}, 0};
    osux_hashtable_each_r(c->ht, (ht_fun) cnte_add_nb_inherit, &h);
    return h.nb;
}

//--------------------------------------------------

void cnt_print(const struct counter *c)
{
    printf("Counter: (%g)\n", c->total);
    printf("Entry:\tkey\tval\tfreq\n");
    osux_hashtable_each_r(c->ht, (ht_fun) cnte_print, NULL);
}

void cnt_print_inherit(const struct counter *c, inherit_fun inherit)
{
    printf("Counter: (%g)\n", c->total);
    printf("Entry:\tkey\tval\tcompr\tfreq\tfreq cp\n");
    struct inheriter h = {c, NULL, {inherit}, INFINITY};
    osux_hashtable_each_r(c->ht, (ht_fun) cnte_print, &h);
}
