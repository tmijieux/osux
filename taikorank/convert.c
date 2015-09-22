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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map/map.h"
#include "map/parser.h"
#include "ho/hit_object.h"
#include "tp/timing_point.h"
#include "hs/hitsound.h"

#include "struct.h"
#include "taiko_ranking_object.h"
#include "convert.h"
#include "print.h"

#define BASIC_SV 1.4

#define TYPE(type)    (type & (~HO_NEWCOMBO) & 0x0F)
// this get rid of the 'new_combo' flag to get the hit object's type
// more easily

//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------

char convert_get_type (struct hit_object * ho)
{
  int type = ho->type;
  int sample = ho->hs.sample;

  if (TYPE(type) == HO_SLIDER)
    {
      if ((sample & HS_FINISH) != 0)
	return 'R';
      else
	return 'r';
    }
  if (TYPE(type) == HO_SPINNER)
    return 's';

  if (TYPE(type) == HO_CIRCLE)
    {
      if ((sample & (HS_WHISTLE | HS_CLAP)) != 0)
	{
	  if ((sample & HS_FINISH) != 0)
	    return 'K';
	  else
	    return 'k';
	}
      else
	{
	  if ((sample & HS_FINISH) != 0)
	    return 'D';
	  else
	    return 'd';
	}
    }

  // uselessness
  return '_';
}

//---------------------------------------------------------------

double convert_get_bpm_app (struct timing_point * tp, double sv)
{
  double sv_multiplication;
  if (tp->uninherited)
    sv_multiplication = 1;
  else
    sv_multiplication = -100. / tp->svm;

  return (mpb_to_bpm(tp->last_uninherited->mpb) *
	  sv_multiplication * (sv / BASIC_SV));
}

//---------------------------------------------------------------

int convert_get_end_offset (struct hit_object * ho, int type,
			    double bpm_app)
{
  if (type == 's')
    {
      return ho->spi.end_offset;
    }
  if (type == 'r' || type == 'R')
    {
      return ho->offset + ((ho->sli.length * ho->sli.repeat) *
			   (MSEC_IN_MINUTE / (100. * BASIC_SV)) /
			   bpm_app);
    }
  // else circle
  return ho->offset;
}

//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------

struct tr_map * convert (char* file_name)
{
  struct map * map = osu_map_parser(file_name);
  
  if (map->Mode != MODE_TAIKO)
    {
      fprintf(OUTPUT_ERR, "Autoconverts are said to be bad. But I don't think so. Please implement the corresponding functions.\n");
      map_free(map);
      return NULL;
    }
  
  struct tr_map * tr_map = malloc(sizeof(struct tr_map));
  tr_map->nb_object = map->hoc;
  tr_map->object = malloc(map->hoc * sizeof(struct tr_object));

  // set last uninherited
  for (int i = 0; i < map->tpc; i++)
    {
      if (map->TimingPoints[i].uninherited)
	map->TimingPoints[i].last_uninherited = &map->TimingPoints[i];
      else
	map->TimingPoints[i].last_uninherited
	    = map->TimingPoints[i-1].last_uninherited;
    }

  // set objects
  int current_tp = 0;
  for (int i = 0; i < map->hoc; i++)
    {
      while (current_tp < (map->tpc - 1) &&
	     map->TimingPoints[current_tp + 1].offset
	     <= map->HitObjects[i].offset)
	  current_tp++;
	
      tr_map->object[i].offset     = (int) map->HitObjects[i].offset;
      tr_map->object[i].type       = convert_get_type(&map->HitObjects[i]);
      tr_map->object[i].bpm_app    = convert_get_bpm_app(&map->TimingPoints[current_tp],
							 map->SliderMultiplier);
      tr_map->object[i].end_offset = convert_get_end_offset(&map->HitObjects[i],
							    tr_map->object[i].type,
							    tr_map->object[i].bpm_app);
    }

  // get other data
  tr_map->od = map->OverallDifficulty;
  tr_map->title         = malloc(sizeof(char) * (1 + strlen(map->Title)));
  tr_map->creator       = malloc(sizeof(char) * (1 + strlen(map->Creator)));
  tr_map->diff          = malloc(sizeof(char) * (1 + strlen(map->Version)));
  strcpy(tr_map->title,         map->Title);
  strcpy(tr_map->creator,       map->Creator);
  strcpy(tr_map->diff,          map->Version);

  map_free(map);
  return tr_map;
}
