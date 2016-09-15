/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
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
#include <stdlib.h>

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"

#include "print.h"
#include "tr_mods.h"

/*
  _COEFF_MS:  od multiplier related to speed
  _COEFF_BPM: bpm multiplier
 */

#define DT_COEFF_MS    (2 / 3.)
#define DT_COEFF_BPM   1.5

#define HT_COEFF_MS    (4 / 3.)
#define HT_COEFF_BPM   (2 / 3.)

#define NM_COEFF_MS    1

// ----------------------------------------

#define HR_COEFF_OD    1.4
#define EZ_COEFF_OD    0.5

#define MAX_OD 10.

// ----------------------------------------

/*
  _COEFF_SPEED: speed multiplier
 */

#define DT_COEFF_SPEED 1.5
#define HT_COEFF_SPEED (2 / 3.)
#define EZ_COEFF_SPEED (2 / 3.) // not sure

#ifdef RESOLUTION_10
  #define HR_COEFF_SPEED   (4 / 3.)
  #define HDHR_COEFF_SPEED (4 / 3.) // not sure
#elif defined RESOLUTION_14
#elif defined RESOLUTION_18
#endif

// ----------------------------------------

#ifdef RESOLUTION_10
  #define NM_NB_OBJ_APP 10.
#elif defined RESOLUTION_14
  #define NM_NB_OBJ_APP 14.
#elif defined RESOLUTION_18
  #define NM_NB_OBJ_APP 18.
#endif
#define NM_NB_OBJ_DIS  0.

#define FL_NB_OBJ_APP1 5.
#define FL_NB_OBJ_APP2 4.
#define FL_NB_OBJ_APP3 3.5
#define FL_START_APP2  100 // if >=
#define FL_START_APP3  200 // if >=

#define HD_NB_OBJ_APP  10.
#define HD_NB_OBJ_DIS  6.

#ifdef RESOLUTION_10
  #define HDHR_NB_OBJ_DIS 8. // not sure
#elif defined RESOLUTION_14
#elif defined RESOLUTION_18
#endif

static void trm_apply_mods_EZ(struct tr_map * map);
static void trm_apply_mods_HT(struct tr_map * map);
static void trm_apply_mods_HR(struct tr_map * map);
static void trm_apply_mods_DT(struct tr_map * map);
static void trm_apply_mods_HD(struct tr_map * map);
static void trm_apply_mods_HDHR(struct tr_map * map);

static void trm_apply_mods_NM(struct tr_map * map);

static int trm_print_one_mod(const struct tr_map * map, int mods, int * i,
                             char * buffer, char * string);

//---------------------------------------------

static int trm_has_mods(const struct tr_map * map, int mods)
{
    return (map->mods & mods) != 0;
}

//---------------------------------------------

static void trm_apply_mods_HR(struct tr_map * map)
{
    map->od *= HR_COEFF_OD;
    if (map->od > MAX_OD)
        map->od = MAX_OD;

    for (int i = 0; i < map->nb_object; i++)
        map->object[i].bpm_app *= HR_COEFF_SPEED;
}

//---------------------------------------------

static void trm_apply_mods_EZ(struct tr_map * map)
{
    map->od *= EZ_COEFF_OD;

    for (int i = 0; i < map->nb_object; i++)
        map->object[i].bpm_app *= EZ_COEFF_SPEED;
}

//---------------------------------------------

static void trm_apply_mods_DT(struct tr_map * map)
{
    map->od_hit_window_mult = DT_COEFF_MS;

    for (int i = 0; i < map->nb_object; i++) {
        map->object[i].bpm_app    *= DT_COEFF_SPEED;
        map->object[i].offset     /= DT_COEFF_BPM;
        map->object[i].end_offset /= DT_COEFF_BPM;
    }
}

//---------------------------------------------

static void trm_apply_mods_HT(struct tr_map * map)
{
    map->od_hit_window_mult = HT_COEFF_MS;

    for (int i = 0; i < map->nb_object; i++) {
        map->object[i].bpm_app    *= HT_COEFF_SPEED;
        map->object[i].offset     /= HT_COEFF_BPM;
        map->object[i].end_offset /= HT_COEFF_BPM;
    }
}

//---------------------------------------------

static void trm_apply_mods_HD(struct tr_map * map)
{
    if (!trm_has_mods(map, MODS_FL))
        for (int i = 0; i < map->nb_object; i++)
            map->object[i].obj_app = HD_NB_OBJ_APP;

    for (int i = 0; i < map->nb_object; i++)
        if (tro_is_circle(&map->object[i]))
            map->object[i].obj_dis = HD_NB_OBJ_DIS;
}

//---------------------------------------------

static void trm_apply_mods_HDHR(struct tr_map * map)
{
    for (int i = 0; i < map->nb_object; i++) {
        map->object[i].bpm_app *= HDHR_COEFF_SPEED;
        if (tro_is_circle(&map->object[i]))
            map->object[i].obj_dis = HDHR_NB_OBJ_DIS;
    }
}

//---------------------------------------------

void trm_apply_mods_FL(struct tr_map * map)
{
    int combo = 0;
    for (int i = 0; i < map->nb_object; i++) {
        if (map->object[i].ps == GREAT || map->object[i].ps == GOOD)
            combo++;
        else if (map->object[i].ps == MISS)
            combo = 0;

        if (combo >= FL_START_APP3)
            map->object[i].obj_app = FL_NB_OBJ_APP3;
        else if (combo >= FL_START_APP2)
            map->object[i].obj_app = FL_NB_OBJ_APP2;
        else
            map->object[i].obj_app = FL_NB_OBJ_APP1;
    }
}

//---------------------------------------------

static void trm_apply_mods_NM(struct tr_map * map)
{
    map->od_hit_window_mult = NM_COEFF_MS;
    for (int i = 0; i < map->nb_object; i++) {
        map->object[i].obj_app = NM_NB_OBJ_APP;
        map->object[i].obj_dis = NM_NB_OBJ_DIS;
    }
}

//---------------------------------------------
//---------------------------------------------
//---------------------------------------------

void trm_apply_mods(struct tr_map * map)
{
    trm_apply_mods_NM(map);

    if (trm_has_mods(map, MODS_HR) && !trm_has_mods(map, MODS_HD))
        trm_apply_mods_HR(map);
    else if (trm_has_mods(map, MODS_EZ))
        trm_apply_mods_EZ(map);

    if (trm_has_mods(map, MODS_DT))
        trm_apply_mods_DT(map);
    else if (trm_has_mods(map, MODS_HT))
        trm_apply_mods_HT(map);

    if (trm_has_mods(map, MODS_HD) && !trm_has_mods(map, MODS_HR))
        trm_apply_mods_HD(map);
    if (trm_has_mods(map, MODS_HD) && trm_has_mods(map, MODS_HR))
        trm_apply_mods_HDHR(map);
    if (trm_has_mods(map, MODS_FL))
        trm_apply_mods_FL(map);
}

//-------------------------------------------------

static int trm_print_one_mod(const struct tr_map * map, int mods, int * i,
                             char * buffer, char * string)
{
    if ((map->mods & mods) != 0) {
        sprintf(&buffer[*i], string);
        *i += STR_MODS_LENGTH;
        return 1;
    }
    return 0;
}

//-------------------------------------------------

void trm_print_out_mods(const struct tr_map * map)
{
    char * buffer = trm_mods_to_str(map);
    print_string_size(buffer, STR_MODS_LENGTH * MAX_MODS + 1, OUTPUT);
    free(buffer);
}

//-------------------------------------------------

char * trm_mods_to_str(const struct tr_map * map)
{
    char * s = calloc(sizeof(char), STR_MODS_LENGTH * MAX_MODS + 1);
    int i = 0;

    if (trm_print_one_mod(map, MODS_HR, &i, s, "HR ") == 0)
        trm_print_one_mod(map, MODS_EZ, &i, s, "EZ ");

    if (trm_print_one_mod(map, MODS_DT, &i, s, "DT ") == 0)
        trm_print_one_mod(map, MODS_HT, &i, s, "HT ");

    trm_print_one_mod(map, MODS_HD, &i, s, "HD ");
    trm_print_one_mod(map, MODS_FL, &i, s, "FL ");

    return s;
}
