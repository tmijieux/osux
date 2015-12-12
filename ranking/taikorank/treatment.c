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

#include <stdlib.h>

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "treatment.h"

static void trm_treatment_hand (struct tr_map * map);
static void trm_treatment_rest (struct tr_map * map);
static void trm_treatment_color_rest (struct tr_map * map);
static void trm_treatment_app_dis_offset (struct tr_map * map);
static void trm_treatment_visible_time (struct tr_map * map);

#define MAX_REST 10000.

// offset app & dis
#define OFFSET_MIN 10000
#define OFFSET_MAX (-obj->obj_dis / obj->obj_app * OFFSET_MIN)

//------------------------------------------------

static void trm_treatment_hand (struct tr_map * map)
{
  int d_hand = 0;
  int k_hand = 0;
  for (int i = 0; i < map->nb_object; i++)
    {
      struct tr_object * obj = &map->object[i];
      if ((tro_is_big(obj) && tro_is_circle(obj)) ||
	  obj->type == 's')
	{
	  obj->l_hand = 1;
	  obj->r_hand = 1;
	}
      else if (tro_is_don(obj))
	{
	  obj->l_hand = d_hand;
	  obj->r_hand = (d_hand = !d_hand);
	}
      else if (tro_is_kat(obj))
	{
	  obj->l_hand = k_hand;
	  obj->r_hand = (k_hand = !k_hand);
	}
      else // 'r' | 'R'
	{
	  obj->l_hand = 0;
	  obj->r_hand = 0;
	}	
    }
}

//------------------------------------------------

static void trm_treatment_rest (struct tr_map * map)
{
  map->object[0].rest = MAX_REST;
  for (int i = 1; i < map->nb_object; i++)
    {
      map->object[i].rest = (map->object[i].offset -
			     map->object[i-1].end_offset);
    }
}

//------------------------------------------------

static void trm_treatment_color_rest (struct tr_map * map)
{
  struct tr_object * last_l = NULL;
  struct tr_object * last_r = NULL;

  for (int i = 0; i < map->nb_object; i++)
    if (map->object[i].l_hand == 1)
      {
	map->object[i].color_rest = MAX_REST;
	last_l = &map->object[i];
	break;
      }
  for (int i = 0; i < map->nb_object; i++)
    if (map->object[i].r_hand == 1)
      {
	map->object[i].color_rest = MAX_REST;
	last_r = &map->object[i];
	break;
      }

  for (int i = 0; i < map->nb_object; i++)
    {
      struct tr_object * obj = &map->object[i];
      if (tro_is_bonus(obj) || obj == last_l || obj == last_r)
	{
	  obj->color_rest = 0;
	  continue;
	}
      
      if(tro_are_same_hand(obj, last_r))
	{
	  obj->color_rest = obj->offset - last_r->end_offset;
	  last_r = obj;
	}
      if(tro_are_same_hand(obj, last_l))
	{
	  obj->color_rest = obj->offset - last_l->end_offset;
	  last_l = obj;
	}
    }
}

//-----------------------------------------------------

static void trm_treatment_app_dis_offset (struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      struct tr_object * obj = &map->object[i];
      double space_unit = mpb_to_bpm(obj->bpm_app) / 4.;
      // wrong for spinner...
      obj->offset_app = (obj->offset -
			 obj->obj_app * space_unit);
      obj->offset_dis = (obj->end_offset -
			 obj->obj_dis * space_unit);

      /*
      // really far numbers... in case of really slow circle
      if (obj->offset_app < map->object[0].offset - OFFSET_MIN)
	{
	  obj->offset_app = map->object[0].offset - OFFSET_MIN;
	  obj->offset_dis = obj->end_offset       + OFFSET_MAX;
	}
      */
      if (tro_is_slider(obj))
	{
	  int diff = obj->end_offset - obj->offset;
	  obj->end_offset_app = obj->offset_app + diff;
	  obj->end_offset_dis = obj->offset_dis + diff;
	}
      else if (obj->type == 's')
	{
	  int diff = obj->end_offset - obj->offset;
	  obj->end_offset_app = obj->offset_app;
	  obj->end_offset_dis = obj->offset_dis + diff;
	}
      else // d D k K
	{
	  obj->end_offset_app = obj->offset_app;
	  obj->end_offset_dis = obj->offset_dis;
	}
    }
}

//-----------------------------------------------------

static void trm_treatment_visible_time (struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      struct tr_object * obj = &map->object[i];
      obj->visible_time = obj->offset_dis - obj->end_offset_app;
      if (obj->type == 's')
	obj->visible_time -= obj->end_offset - obj->offset;
      obj->invisible_time = obj->end_offset - obj->offset_dis;

      if (obj->visible_time < 0)
	{
	  // object will never appear
	  obj->visible_time = 0;
	  obj->invisible_time = 0;
	}
    }
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_treatment (struct tr_map * map)
{
  trm_treatment_hand(map);
  trm_treatment_rest(map);
  trm_treatment_color_rest(map);
  trm_treatment_app_dis_offset(map);
  trm_treatment_visible_time(map);
}
