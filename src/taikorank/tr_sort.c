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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "tr_sort.h"

#define TRO_COMPARE(FIELD)                              \
    static int tro_compare_##FIELD(const void * o1,     \
                                   const void * o2)     \
    {                                                   \
        const struct tr_object * obj1 = o1;             \
        const struct tr_object * obj2 = o2;             \
        double f = obj1->FIELD - obj2->FIELD;           \
        return f < 0 ? -1 : (f > 0 ? 1 : 0);            \
    }

#define TRM_SORT(FIELD)                                         \
    void tro_sort_##FIELD (struct tr_object * o, int nb)        \
    {                                                           \
        qsort(o, nb, sizeof(struct tr_object),                  \
              tro_compare_##FIELD);                             \
    }                                                           \
    void trm_sort_##FIELD (struct tr_map * map)                 \
    {                                                           \
        tro_sort_##FIELD (map->object, map->nb_object);         \
    }

#define TRM_SORT_FUNCTIONS(FIELD)               \
    TRO_COMPARE(FIELD)                          \
    TRM_SORT(FIELD)

TRM_SORT_FUNCTIONS(offset)
TRM_SORT_FUNCTIONS(density_star)
TRM_SORT_FUNCTIONS(reading_star)
TRM_SORT_FUNCTIONS(pattern_star)
TRM_SORT_FUNCTIONS(accuracy_star)
TRM_SORT_FUNCTIONS(final_star)
