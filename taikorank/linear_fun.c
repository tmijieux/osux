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
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "print.h"
#include "cst_yaml.h"
#include "vector.h"
#include "linear_fun.h"

#define ERROR_VAL INFINITY

struct linear_fun * lf_new(struct vector * v)
{
    struct linear_fun * lf = malloc(sizeof(*lf));
    lf->len = v->len;
    lf->x = malloc(sizeof(double) * lf->len);
    for(int i = 0; i < lf->len; i++) {
	lf->x[i] = v->t[i][0];
    }
    lf->a = malloc(sizeof(double) * lf->len - 1);
    lf->b = malloc(sizeof(double) * lf->len - 1);
    for(int i = 0; i < lf->len - 1; i++) {
	lf->a[i] = ((v->t[i][1] - v->t[i+1][1]) / 
		    (v->t[i][0] - v->t[i+1][0]));
	lf->b[i] = v->t[i][1] - lf->a[i] * v->t[i][0];
    }
    return lf;
}

void lf_free(struct linear_fun * lf)
{
    if(lf == NULL)
	return;
    free(lf->x);
    free(lf->a);
    free(lf->b);
    free(lf);
}

//--------------------------------------------------

double lf_eval(struct linear_fun * lf, double x)
{
    if (x < lf->x[0]) {
	tr_error("Inf out of bounds value (%g) for linear_fun (%s) eval.",
		 x, lf->name);
	return ERROR_VAL;
    }
    for(int i = 1; i < lf->len; i++) {
	if (x <= lf->x[i]) {
	    return lf->a[i-1] * x + lf->b[i-1];
	}
    }
    tr_error("Sup out of bounds value (%g) for linear_fun (%s) eval.", x, lf->name);
    return ERROR_VAL;
}

//--------------------------------------------------

struct linear_fun * cst_lf(struct hash_table * ht, const char * key)
{
    struct vector * v = cst_vect(ht, key);
    struct linear_fun * lf = lf_new(v);
    vect_free(v);
    lf->name = (char*) key;
    return lf;
}

