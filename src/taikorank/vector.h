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
#ifndef TR_VECTOR_H
#define TR_VECTOR_H

struct vector {
    int len;
    int max_index;
    int min_index;
    double **t;
};

struct vector *vect_new(int length, int dim);
void vect_free(struct vector *v);

struct vector *cst_vect(osux_hashtable *ht, const char *key);
struct vector *cst_vect_from_decl(osux_hashtable *ht, const char *key);
struct vector *cst_vect_from_list(osux_hashtable *ht, const char *key);

#endif // TR_VECTOR_H
