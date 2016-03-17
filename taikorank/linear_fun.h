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
#ifndef LINEAR_FUN_H
#define LINEAR_FUN_H

struct vector;

// piecewise linear function
// f(x) = a*x+b
struct linear_fun {
    char * name;
    int len;
    double * x;
    double * a;
    double * b;
};

struct linear_fun * lf_new(struct vector * v);
void lf_free(struct linear_fun * lf);

// Values must be sorted 
struct linear_fun * cst_lf(struct hash_table * ht, const char * key);
double lf_eval(struct linear_fun * lf, double x);

#endif //LINEAR_FUN_H
