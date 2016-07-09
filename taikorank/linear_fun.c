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

#include "util/string2.h"

#include "print.h"
#include "cst_yaml.h"
#include "vector.h"
#include "linear_fun.h"

#define ERROR_VAL INFINITY

// piecewise linear function
// f(x) = a*x+b
struct linear_fun {
    char * name;
    int len;
    double * x; // len
    double * a; // len - 1
    double * b; // len - 1
};
/*
  x = [x0, x1, x2, ...]
  a = [a0, a1, a2, ...]
  b = [b0, b1, b2, ...]
  in [xi, xi+1] use ai, bi
 */

//--------------------------------------------------

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

static inline int find_interval_binary(const double * array,
				       int l, int r, double x)
{
    if (r < l)
	return -1;
    int m = (l + r) / 2;
    if (x < array[m])
	return find_interval_binary(array, l, m-1, x);
    if (array[m+1] < x)
	return find_interval_binary(array, m+1, r, x);
    return m;
}

static inline int find_interval_linear(const double * array, int len,
				       double x)
{
    if (x < array[0])
	return -1;
    for(int i = 1; i < len; i++)
	if (x <= array[i])
	    return i-1;
    return -1;
}

static inline double lf_eval_interval(struct linear_fun * lf, 
				      double x, int i)
{
    return lf->a[i] * x + lf->b[i];
}

double lf_eval(struct linear_fun * lf, double x)
{
    // As array are small, binary search is not really faster
    int i = find_interval_linear(lf->x, lf->len, x);
    //int i = find_interval_binary(lf->x, 0, lf->len-1, x);
    if (i < 0) {
	tr_error("Out of bounds value (%g) for linear_fun (%s) eval",
		 x, lf->name);
	lf_print(lf);
	return ERROR_VAL;
    }
    return lf_eval_interval(lf, x, i);
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

//--------------------------------------------------

void lf_print(struct linear_fun * lf)
{
    fprintf(stderr, "linear_fun: %s\nx:", lf->name);
    for (int i = 0; i < lf->len; i++) 
	fprintf(stderr, "\t%.4g", lf->x[i]);
    fprintf(stderr, "\na:");
    for (int i = 0; i < lf->len-1; i++) 
	fprintf(stderr, "\t%.4g", lf->a[i]);
    fprintf(stderr, "\nb:");
    for (int i = 0; i < lf->len-1; i++) 
	fprintf(stderr, "\t%.4g", lf->b[i]);
    fprintf(stderr, "\n");
}

//--------------------------------------------------

static char * lf_array_to_str(double * t, int len, char * var)
{
    char * s = xasprintf("%g", t[0]);
    for (int i = 1; i < len; i++) {
	s = xasprintf("%s, %g", s, t[i]);
    }
    return xasprintf("double %s[%d] = {%s};\n", var, len, s);
}

void lf_dump(struct linear_fun * lf)
{
    char * x_name = xasprintf("%s_x", lf->name);
    char * x = lf_array_to_str(lf->x, lf->len,   x_name);
    char * a_name = xasprintf("%s_a", lf->name);
    char * a = lf_array_to_str(lf->a, lf->len-1, a_name);
    char * b_name = xasprintf("%s_b", lf->name);
    char * b = lf_array_to_str(lf->b, lf->len-1, b_name);
    char * var = xasprintf("struct linear_fun lf_%s = {\n"
			   "\t.name = \"%s\",\n"
			   "\t.len = %d,\n"
			   "\t.x = %s,\n"
			   "\t.a = %s,\n"
			   "\t.b = %s,\n"
			   "};\n",
			   lf->name, lf->name, lf->len,
			   x_name, a_name, b_name);
    char * all = xasprintf("%s%s%s%s", x, a, b, var);
    fprintf(stderr, "%s", all);
}
