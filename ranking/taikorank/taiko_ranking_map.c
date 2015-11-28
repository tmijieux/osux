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

#include "beatmap/beatmap.h"
#include "beatmap/parser/parser.h"
#include "beatmap/hitobject.h"
#include "beatmap/timingpoint.h"
#include "beatmap/hitsound.h"

#include "check_osu_file.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"

#include "print.h"

#include "tr_db.h"

#include "mods.h"
#include "treatment.h"

#include "density.h"
#include "reading.h"
#include "pattern.h"
#include "accuracy.h"
#include "final_star.h"

#define BASIC_SV 1.4

#define TYPE(type)    (type & (~HO_NEWCOMBO) & 0x0F)
// this get rid of the 'new_combo' flag to get the hit object's type
// more easily

static char convert_get_type (struct hit_object * ho);
static double convert_get_bpm_app (struct timing_point * tp,
				   double sv);
static int convert_get_end_offset (struct hit_object * ho, int type,
				   double bpm_app);
static struct tr_map * trm_convert (char* file_name);

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

void trm_main(const struct tr_map * map, int mods)
{
  struct tr_map * map_copy = trm_copy(map);
  trm_set_mods(map_copy, mods);

  // compute
  trm_compute_stars(map_copy);
  
  // printing
  trm_print_tro(map_copy, FILTER_APPLY);
  trm_print(map_copy);

  // db
  tr_db_add(map_copy);
  
  // free
  trm_free(map_copy);
}

//--------------------------------------------------

void trm_set_mods(struct tr_map * map, int mods)
{
  map->mods = mods;
}

//--------------------------------------------------

void trm_compute_stars(struct tr_map * map)
{
  trm_apply_mods(map);
  trm_treatment(map);
  
  trm_compute_density(map);
  trm_compute_reading(map);
  trm_compute_pattern(map);
  trm_compute_accuracy(map);
  trm_compute_final_star(map);
}

//--------------------------------------------------

struct tr_map * trm_copy(const struct tr_map * map)
{
  struct tr_map * copy = calloc(sizeof(*copy), 1);
  memcpy(copy, map, sizeof(*map));

  copy->object = calloc(sizeof(map->object[0]), map->nb_object);
  memcpy(copy->object, map->object,
	 sizeof(map->object[0]) * map->nb_object);
  
  copy->title   = strdup(map->title);
  copy->artist  = strdup(map->artist);
  copy->source  = strdup(map->source);
  copy->creator = strdup(map->creator);
  copy->diff    = strdup(map->diff);
  /*copy->title_uni   = strdup(map->title_uni);
    copy->artist_uni  = strdup(map->artist_uni);*/
  
  return copy;
}

//-----------------------------------------------------

void trm_pattern_free(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      free(map->object[i].alt);
      free(map->object[i].singletap);
    }  
}

//--------------------------------------------------

void trm_free(struct tr_map * map)
{
  trm_pattern_free(map);
  free(map->title);
  free(map->artist);
  free(map->source);
  free(map->creator);
  free(map->diff);
  /*free(map->title_uni);
    free(map->artist_uni);*/
  
  free(map->object);
  free(map);
}

//--------------------------------------------------

struct tr_map * trm_new(char * file_name)
{
  if (check_file(file_name) == 0)
    return NULL;

  return trm_convert(file_name);
}

//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------

static char convert_get_type (struct hit_object * ho)
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

static double convert_get_bpm_app (struct timing_point * tp,
				   double sv)
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

static int convert_get_end_offset (struct hit_object * ho, int type,
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

struct tr_map * trm_convert (char* file_name)
{
  struct map * map = osu_map_parser(file_name);
  
  if (map->Mode != MODE_TAIKO)
    {
      fprintf(OUTPUT_ERR, "Autoconverts are said to be bad. But "
	      "I don't think so. Please implement the corresponding"
	      " functions.\n");
      map_free(map);
      return NULL;
    }
  
  struct tr_map * tr_map = calloc(sizeof(struct tr_map), 1);
  tr_map->nb_object = map->hoc;
  tr_map->object = calloc(sizeof(struct tr_object), map->hoc);

  // set last uninherited
  for (int i = 0; i < map->tpc; i++)
    {
      if (map->TimingPoints[i].uninherited)
	map->TimingPoints[i].last_uninherited =
	  &map->TimingPoints[i];
      else
	map->TimingPoints[i].last_uninherited =
	  map->TimingPoints[i-1].last_uninherited;
    }

  // set objects
  int current_tp = 0;
  for (int i = 0; i < map->hoc; i++)
    {
      while (current_tp < (map->tpc - 1) &&
	     map->TimingPoints[current_tp + 1].offset
	     <= map->HitObjects[i].offset)
	  current_tp++;
	
      tr_map->object[i].offset     =
	(int) map->HitObjects[i].offset;
      tr_map->object[i].type       =
	convert_get_type(&map->HitObjects[i]);
      tr_map->object[i].bpm_app    =
	convert_get_bpm_app(&map->TimingPoints[current_tp],
			    map->SliderMultiplier);
      tr_map->object[i].end_offset =
	convert_get_end_offset(&map->HitObjects[i],
			       tr_map->object[i].type,
			       tr_map->object[i].bpm_app);
    }

  // get other data
  tr_map->od = map->OverallDifficulty;
  tr_map->title      = strdup(map->Title);
  tr_map->artist     = strdup(map->Artist);
  tr_map->source     = strdup(map->Source);
  tr_map->creator    = strdup(map->Creator);
  tr_map->diff       = strdup(map->Version);
  tr_map->bms_osu_ID = map->BeatmapSetID;
  /*tr_map->diff_osu_ID = map->BeatmapID;
  tr_map->title_uni  = strdup(map->TitleUnicode);
  tr_map->artist_uni = strdup(map->ArtistUnicode);*/
  
  map_free(map);
  return tr_map;
}

//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------

void trm_print_tro(struct tr_map * map, int filter)
{
  if ((filter & FILTER_BASIC) != 0)
    fprintf(OUTPUT_INFO, "offset\trest\ttype\tbpm app\t");
  if ((filter & FILTER_BASIC_PLUS) != 0)
    fprintf(OUTPUT_INFO, "offset\tend\trest\trest2\ttype\tbpm app\t");
  if ((filter & FILTER_ADDITIONNAL) != 0)
    fprintf(OUTPUT_INFO, "l hand\tr hand\trest\tobj app\tobjdis\t");
  if ((filter & FILTER_DENSITY) != 0)
    fprintf(OUTPUT_INFO, "dnst rw\tdnst cl\tdnst*\t");
  if ((filter & FILTER_READING) != 0)
    fprintf(OUTPUT_INFO, "app\tdis\tvisi\tinvisi\tsperpos\thidden\thide\tslow\tfast\tspd chg\tread*\t");
  if ((filter & FILTER_READING_PLUS) != 0)
    fprintf(OUTPUT_INFO, "app\tend app\tdis\tend dis\tsperpos\thidden\thide\tspeed\tspd chg\tread*\t");
  if ((filter & FILTER_PATTERN) != 0)
    fprintf(OUTPUT_INFO, "proba\talt1\talt2\tsingle1\tsingle2\tpttrn*\t");
  if ((filter & FILTER_ACCURACY) != 0)
    fprintf(OUTPUT_INFO, "%g\t%g\t", map->great_ms, map->bad_ms);
  
  fprintf(OUTPUT_INFO, "\n");
  
  for (int i = 0; i < map->nb_object; ++i)
    tro_print(&map->object[i], filter);
}

//-------------------------------------------------

void trm_print(struct tr_map * map)
{
  fprintf(OUTPUT, "%.4g\t", map->pattern_star);
  fprintf(OUTPUT, "%.4g\t", map->density_star);
  
  fprintf(OUTPUT, "%.4g\t", map->final_star);
  /*fprintf(OUTPUT, "%.4g\t", map->reading_star);
  fprintf(OUTPUT, "%.4g\t", map->accuracy_star);
  */
  trm_print_mods(map);
  print_string_size(map->diff,    24, OUTPUT);
  print_string_size(map->title,   32, OUTPUT);
  print_string_size(map->creator, 16, OUTPUT);
  fprintf(OUTPUT, "\n");
}

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

int trm_hardest_tro(struct tr_map * map)
{
  int best = 0;
  for(int i = 0; i < map->nb_object; i++)
    if(map->object[i].final_star > map->object[best].final_star)
      best = i;
  return best;
}

//--------------------------------------------------

void trm_remove_tro(struct tr_map * map, int x)
{
  for(int i = x; i < map->nb_object - 1; i++)
    map->object[i] = map->object[i+1];
  map->nb_object--;
}
