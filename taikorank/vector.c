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
//#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "util/list.h"
#include "util/string2.h"
#include "cst_yaml.h"
#include "vector.h"
#include "interpolation.h"

#define CST_VECT_DIM 2

//--------------------------------------------------

struct vector * vect_new(int length, int dim)
{
    if(length < 0)
	length = 0;
    struct vector * v = malloc(sizeof(*v));
    v->len = length;
    v->t = malloc(sizeof(double *) * v->len);
    for(int i = 0; i < v->len; i++)
	v->t[i] = malloc(sizeof(double) * dim);
    return v;
}

//--------------------------------------------------

void vect_free(struct vector * v)
{
    if(v == NULL)
	return;
    for(int i = 0; i < v->len; i++)
	free(v->t[i]);
    free(v->t);
    free(v);
}

//--------------------------------------------------

struct vector * cst_vect2(struct hash_table * ht, const char * key)
{
    struct osux_list * l = cst_list(ht, key);
    struct vector * v = vect_new(osux_list_size(l), CST_VECT_DIM);
    for(int i = 0; i < v->len; i++) {
	struct osux_list * l2 = yw_extract_list(osux_list_get(l, i+1));
	for(int j = 0; j < CST_VECT_DIM; j++)
	    v->t[i][j] = atof(yw_extract_scalar(osux_list_get(l2, j+1)));
    }
    return v;
}

struct vector * cst_vect(struct hash_table * ht, const char * key)
{
    char *s = xasprintf("%s_length", key);
    struct vector * v = vect_new(cst_i(ht, s), CST_VECT_DIM);
    free(s);
    v->max_index = 0;
    v->min_index = 0;
    for(int i = 0; i < v->len; i++) {
	for(int j = 0; j < CST_VECT_DIM; j++) {
	    s = xasprintf("%s_%c%d", key, 'x'+j, i+1);
	    v->t[i][j] = cst_f(ht, s);
	    free(s);
	}
	if(v->t[i][0] > v->t[v->max_index][0])
	    v->max_index = i;
	if(v->t[i][0] < v->t[v->min_index][0])
	    v->min_index = i;
    }
    return v;
}

//--------------------------------------------------

double vect_exp(struct vector * v, double x)
{
    return EXP_2_PT(x, 
		    v->t[0][0], v->t[0][1],
		    v->t[1][0], v->t[1][1]);
}

double vect_poly2(struct vector * v, double x)
{
    if(x > v->t[v->max_index][0]) {
	return v->t[v->max_index][1];
    }
    if(x < v->t[v->min_index][0]) {
	return v->t[v->min_index][1];
    }

    return POLY_2_PT(x, 
		     v->t[0][0], v->t[0][1],
		     v->t[1][0], v->t[1][1]);
}

