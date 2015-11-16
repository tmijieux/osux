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

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"

#include "mods.h"

static void trm_apply_mods_EZ (struct tr_map * map);
static void trm_apply_mods_HT (struct tr_map * map);
static void trm_apply_mods_HR (struct tr_map * map);
static void trm_apply_mods_DT (struct tr_map * map);
static void trm_apply_mods_HD (struct tr_map * map);
static void trm_apply_mods_FL (struct tr_map * map);

static void trm_apply_NM_app_dis(struct tr_map * map);
static void trm_apply_NM_ms_coeff(struct tr_map * map);

//---------------------------------------------

static void trm_apply_mods_HR (struct tr_map * map)
{
  map->od *= HR_COEFF_OD;
  if (map->od > MAX_OD)
    map->od = MAX_OD;
    
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].bpm_app *= HR_COEFF_SPEED; 
    }
}

//---------------------------------------------

static void trm_apply_mods_EZ (struct tr_map * map)
{
  map->od *= EZ_COEFF_OD;
  
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].bpm_app *= EZ_COEFF_SPEED; 
    }
}

//---------------------------------------------

static void trm_apply_mods_DT (struct tr_map * map)
{
  map->great_ms = DT_COEFF_MS;
  map->bad_ms   = DT_COEFF_MS;
  
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].bpm_app    *= DT_COEFF_SPEED;
      map->object[i].offset     /= DT_COEFF_BPM;
      map->object[i].end_offset /= DT_COEFF_BPM; 
    }
}

//---------------------------------------------

static void trm_apply_mods_HT (struct tr_map * map)
{
  map->great_ms = HT_COEFF_MS;
  map->bad_ms   = HT_COEFF_MS;
  
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].bpm_app    *= HT_COEFF_SPEED;
      map->object[i].offset     /= HT_COEFF_BPM;
      map->object[i].end_offset /= HT_COEFF_BPM; 
    }
}

//---------------------------------------------

static void trm_apply_mods_HD (struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      if (tro_is_circle(&map->object[i]))
	{
	  if ((map->mods & MODS_FL) == 0)
	    map->object[i].obj_app = HD_NB_OBJ_APP;

	  map->object[i].obj_dis = HD_NB_OBJ_DIS;
	}
      else
	{
	  if ((map->mods & MODS_FL) == 0)
	    map->object[i].obj_app = HD_NB_OBJ_APP;
	  map->object[i].obj_dis = NM_NB_OBJ_DIS;
	}
    }
}

//---------------------------------------------

static void trm_apply_mods_FL (struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      if (i >= FL_START_APP3)
	map->object[i].obj_app = FL_NB_OBJ_APP3;
      else if (i >= FL_START_APP2)
	map->object[i].obj_app = FL_NB_OBJ_APP2;
      else 
	map->object[i].obj_app = FL_NB_OBJ_APP1;
      
      if ((map->mods & MODS_HD) == 0)
	map->object[i].obj_dis = NM_NB_OBJ_DIS;
    }
}

//---------------------------------------------

static void trm_apply_NM_app_dis(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].obj_app = NM_NB_OBJ_APP;
      map->object[i].obj_dis = NM_NB_OBJ_DIS;
    }
}

//---------------------------------------------

static void trm_apply_NM_ms_coeff(struct tr_map * map)
{
  map->great_ms = NM_COEFF_MS;
  map->bad_ms   = NM_COEFF_MS;
}

//---------------------------------------------
//---------------------------------------------
//---------------------------------------------

void trm_apply_mods (struct tr_map * map)
{  
  if ((map->mods & MODS_HR) != 0)
    trm_apply_mods_HR(map);
  else if ((map->mods & MODS_EZ) != 0)
    trm_apply_mods_EZ(map);
  
  if ((map->mods & MODS_DT) != 0)
    trm_apply_mods_DT(map);
  else if ((map->mods & MODS_HT) != 0)
    trm_apply_mods_HT(map);

  if ((map->mods & MODS_HD) != 0)
    trm_apply_mods_HD(map);
  if ((map->mods & MODS_FL) != 0)
    trm_apply_mods_FL(map);

  if ((map->mods & (MODS_FL | MODS_HD)) == 0)
    trm_apply_NM_app_dis(map);
  if ((map->mods & (MODS_DT | MODS_HT)) == 0)
    trm_apply_NM_ms_coeff(map);
}

//---------------------------------------------