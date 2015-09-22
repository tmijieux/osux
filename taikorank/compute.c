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

#include "struct.h"
#include "compute.h"
#include "taiko_ranking_object.h"

void compute_hand (struct tr_map * map)
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

void compute_rest (struct tr_map * map)
{
  map->object[0].rest = 0;
  for (int i = 1; i < map->nb_object; i++)
    {
      map->object[i].rest = (map->object[i].offset -
			     map->object[i-1].end_offset);
    }
}
