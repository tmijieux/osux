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
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "osux.h"

#include "cst_yaml.h"
#include "vector.h"
#include "print.h"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#define CST_VECT_DIM 2

//--------------------------------------------------

struct vector * vect_new(int length, int dim)
{
    if (length < 0)
        length = 0;
    struct vector * v = malloc(sizeof(*v));
    v->len = length;
    v->t = malloc(sizeof(double *) * v->len);
    for (int i = 0; i < v->len; i++)
        v->t[i] = malloc(sizeof(double) * dim);
    return v;
}

//--------------------------------------------------

void vect_free(struct vector * v)
{
    if (v == NULL)
        return;
    for (int i = 0; i < v->len; i++)
        free(v->t[i]);
    free(v->t);
    free(v);
}

//--------------------------------------------------

struct vector * cst_vect_from_list(GHashTable * ht, const char * key)
{
    GList * l = cst_list(ht, key);
    if (l == NULL)
        return NULL;
    // TODO vérifie si 'g_list_length' est genant (traverse la liste)
    struct vector * v = vect_new(g_list_length(l), CST_VECT_DIM);

    GList *it;
    int i = 0, j = 0;
    for (it = l; it != NULL; it = it->next) {
        GList * l2 = yw_extract_list((osux_yaml*) it->data);
        if (l2 == NULL)
            return NULL; // FIXME pas de free? c'est une fuite ça non?

        // TODO idem ici
        g_assert(g_list_length(l2) == CST_VECT_DIM);
        GList *it2;
        for (it2 = l2; it2 != NULL; it2 = it2->next) {
            v->t[i][j] = atof(yw_extract_scalar((osux_yaml*)it2->data));
            ++ j;
        }
        ++ i;
    }
    return v;
}

struct vector * cst_vect_from_decl(GHashTable * ht, const char * key)
{
    char *s = g_strdup_printf("%s_length", key);
    struct vector * v = vect_new(cst_i(ht, s), CST_VECT_DIM);
    g_free(s);
    v->max_index = 0;
    v->min_index = 0;
    for (int i = 0; i < v->len; i++) {
        for (int j = 0; j < CST_VECT_DIM; j++) {
            s = g_strdup_printf("%s_%c%d", key, 'x'+j, i+1);
            v->t[i][j] = cst_f(ht, s);
            g_free(s);
        }
        if (v->t[i][0] > v->t[v->max_index][0])
            v->max_index = i;
        if (v->t[i][0] < v->t[v->min_index][0])
            v->min_index = i;
    }
    return v;
}

//--------------------------------------------------

struct vector * cst_vect(GHashTable * ht, const char * key)
{
    typedef struct vector* (*cst_vect_f)(GHashTable*, const char*);
    static cst_vect_f funs[] = {
        cst_vect_from_decl,
        cst_vect_from_list,
    };
    tr_set_print_level(NONE);
    struct vector * v = NULL;
    for (unsigned int i = 0; i < ARRAY_LENGTH(funs); i++) {
        v = funs[i](ht, key);
        if (v != NULL)
            break;
    }
    tr_set_print_level(ALL);
    if (v == NULL)
        tr_error("Failed to find '%s'", key);
    return v;
}
