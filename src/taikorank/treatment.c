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

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "treatment.h"

static void tro_set_hand(struct tr_object *obj,
                         int *d_hand, int *k_hand);

static void trm_set_length(struct tr_map *map);

#define MAX_REST 10000.

//------------------------------------------------

static void tro_set_hand(struct tr_object *o,
                         int *d_hand, int *k_hand)
{
    if (o->ps == MISS)
        return;

    if ((tro_is_big(o) && tro_is_circle(o)) || tro_is_spinner(o))
        o->bf |= TRO_HAND;
    else {
        if (tro_is_don(o)) {
            if ((*d_hand = !*d_hand)) // change the value and check
                o->bf |= TRO_RH;
            else
                o->bf |= TRO_LH;
        } else if (tro_is_kat(o)) {
            if ((*k_hand = !*k_hand)) // change the value and check
                o->bf |= TRO_RH;
            else
                o->bf |= TRO_LH;
        }
    }
    // r R let 0
}

//------------------------------------------------

void tro_set_length(struct tr_object *obj)
{
    obj->length = obj->end_offset - obj->offset;
}

//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

void trm_set_hand(struct tr_map *map)
{
    int d_hand = 0;
    int k_hand = 0;
    for (int i = 0; i < map->nb_object; i++)
        tro_set_hand(&map->object[i], &d_hand, &k_hand);
}

//------------------------------------------------

void trm_set_rest(struct tr_map *map)
{
    map->object[0].rest = MAX_REST;
    for (int i = 1; i < map->nb_object; i++) {
        if (map->object[i].ps == MISS)
            map->object[i].rest = MAX_REST;
        else if (map->object[i-1].ps == MISS)
            map->object[i].rest = MAX_REST;
        else
            map->object[i].rest = (map->object[i].offset -
                                   map->object[i-1].end_offset);
    }
}

//-----------------------------------------------------

void trm_set_combo(struct tr_map *map)
{
    int combo = 0;
    map->combo = 0;
    for (int i = 0; i < map->nb_object; i++) {
        if (map->object[i].ps == GREAT ||
           map->object[i].ps == GOOD)
            combo++;
        else if (map->object[i].ps == MISS) {
            if (combo > map->combo)
                map->combo = combo;
            combo = 0;
        }
    }
    if (combo > map->combo)
        map->combo = combo;
}

//-----------------------------------------------------

static void trm_set_length(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_set_length(&map->object[i]);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_treatment(struct tr_map *map)
{
    trm_set_length(map);

    trm_set_hand(map);
    trm_set_rest(map);
    trm_set_combo(map);
}
