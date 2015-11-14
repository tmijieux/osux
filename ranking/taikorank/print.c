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

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"

#include "print.h"
#include "mods.h"

static void print_one_tr_object (struct tr_object * obj, int filter);
static int print_one_mod (struct tr_map * map, int mods, int * i,
		   char * buffer, char * string);
static void print_string_size(char *s, int max, FILE * output);

//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------

void print_all_tr_object (struct tr_map * map, int filter)
{
  if ((filter & FILTER_BASIC) != 0)
    fprintf(OUTPUT_INFO, "offset\trest\ttype\tbpm app\t");
  if ((filter & FILTER_BASIC_PLUS) != 0)
    fprintf(OUTPUT_INFO, "offset\tend\trest\ttype\tbpm app\t");
  if ((filter & FILTER_ADDITIONNAL) != 0)
    fprintf(OUTPUT_INFO, "l hand\tr hand\trest\tobj app\tobjdis\t");
  if ((filter & FILTER_DENSITY) != 0)
    fprintf(OUTPUT_INFO, "dnst rw\tdnst cl\tdnst*\t");
  if ((filter & FILTER_READING) != 0)
    fprintf(OUTPUT_INFO, "app\tdis\tvisi\tinvisi\tsperpos\thidden\thide\tspeed\tspd chg\tread*\t");
  if ((filter & FILTER_READING_PLUS) != 0)
    fprintf(OUTPUT_INFO, "app\tend app\tdis\tend dis\tsperpos\thidden\thide\tspeed\tspd chg\tread*\t");
  if ((filter & FILTER_PATTERN) != 0)
    fprintf(OUTPUT_INFO, "alt1\talt2\t");
  if ((filter & FILTER_ACCURACY) != 0)
    fprintf(OUTPUT_INFO, "%g\t%g\t", map->great_ms, map->bad_ms);
  
  fprintf(OUTPUT_INFO, "\n");
  
  for (int i = 0; i < map->nb_object; ++i)
    {
      print_one_tr_object(&map->object[i], filter);
    }
}

//-------------------------------------------------

static void print_one_tr_object (struct tr_object * obj, int filter)
{
  if ((filter & FILTER_BASIC) != 0)
    fprintf(OUTPUT_INFO, "%d\t%d\t%c\t%.3g\t",
	    obj->offset,
	    obj->rest,
	    obj->type,
	    obj->bpm_app);
  if ((filter & FILTER_BASIC_PLUS) != 0)
    fprintf(OUTPUT_INFO, "%d\t%d\t%d\t%c\t%.3g\t",
	    obj->offset,
	    obj->end_offset,
	    obj->rest,
	    obj->type,
	    obj->bpm_app);
  if ((filter & FILTER_ADDITIONNAL) != 0)
    fprintf(OUTPUT_INFO, "%d\t%d\t%g\t%g\t",
	    obj->l_hand,
	    obj->r_hand,
	    obj->obj_app,
	    obj->obj_dis);
  if ((filter & FILTER_DENSITY) != 0)
    fprintf(OUTPUT_INFO, "%g\t%g\t%g\t",
	    obj->density_raw,
	    obj->density_color,
	    obj->density_star);
  if ((filter & FILTER_READING) != 0)
    fprintf(OUTPUT_INFO, "%d\t%d\t%d\t%d\t%g\t%.3g\t%.3g\t%g\t%.2g\t%g\t",
	    obj->offset_app,
	    obj->offset_dis,
	    obj->visible_time,
	    obj->invisible_time,
	    obj->superposed,
	    obj->hidden,
	    obj->hide,
	    obj->speed,
	    obj->speed_change,
	    obj->reading_star);
  if ((filter & FILTER_READING_PLUS) != 0)
    fprintf(OUTPUT_INFO, "%d\t%d\t%d\t%d\t%g\t%.3g\t%.3g\t%g\t%.2g\t%g\t",
	    obj->offset_app,
	    obj->end_offset_app,
	    obj->offset_dis,
	    obj->end_offset_dis,
	    obj->superposed,
	    obj->hidden,
	    obj->hide,
	    obj->speed,
	    obj->speed_change,
	    obj->reading_star);
  if ((filter & FILTER_PATTERN) != 0)
    fprintf(OUTPUT_INFO, "%g\t%g\t",
	    obj->pattern_full_alt1,
	    obj->pattern_full_alt2);
  if ((filter & FILTER_ACCURACY) != 0)
    fprintf(OUTPUT_INFO, "\t");
  if ((filter & FILTER_STAR) != 0)
    fprintf(OUTPUT_INFO, "%g\t%g\t",
	    obj->density_star,
  	    obj->reading_star);
  
  fprintf(OUTPUT_INFO, "\n");
}

//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------

void print_map_star (struct tr_map * map)
{
  fprintf(OUTPUT_INFO, "Density star: \t%.15g\n",
	  map->density_star);
  fprintf(OUTPUT_INFO, "Reading star: \t%.15g\n",
	  map->reading_star);
  /*
    fprintf(OUTPUT_INFO, "Pattern star: \t%g\n",
    map->pattern_star);
  
    fprintf(OUTPUT_INFO, "Accuracy star: \t%g\n",
    map->accuracy_star);
  */
}

//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------

static void print_string_size(char *s, int max, FILE * output)
{
  int length = strlen(s);
  if (length >= max)
    {
      s[max-1] = '\0';
      s[max-2] = '.';
      s[max-3] = '.';
      s[max-4] = '.';
      length = max;
    }
  fprintf(output, "%s", s);
  length = max - length - 1;
  for (int i = 0; i < length; i++)
    fprintf(output, " ");
  fprintf(output, "\t");
}

//-------------------------------------------------

static int print_one_mod (struct tr_map * map, int mods, int * i,
			  char * buffer, char * string)
{
  if ((map->mods & mods) != 0)
    {
      sprintf(&buffer[*i], string);
      *i += STR_MODS_LENGTH;
      return 1;
    }
  return 0;
}

//-------------------------------------------------

void print_mods (struct tr_map * map)
{
  char buffer[STR_MODS_LENGTH * MAX_MODS + 1] = { 0 };
  int i = 0;
  
  if (print_one_mod(map, MODS_HR, &i, buffer, "HR ") == 0)
    print_one_mod(map, MODS_EZ, &i, buffer, "EZ ");

  if (print_one_mod(map, MODS_DT, &i, buffer, "DT ") == 0)
    print_one_mod(map, MODS_HT, &i, buffer, "HT ");

  print_one_mod(map, MODS_HD, &i, buffer, "HD ");
  print_one_mod(map, MODS_FL, &i, buffer, "FL ");

  print_string_size(buffer, STR_MODS_LENGTH * MAX_MODS + 1, OUTPUT);
}

//-------------------------------------------------

void print_map_final (struct tr_map * map)
{
  fprintf(OUTPUT, "%g      \t", map->density_star);
  fprintf(OUTPUT, "%g      \t", map->reading_star);
  print_mods(map);
  print_string_size(map->diff,    24, OUTPUT);
  print_string_size(map->title,   32, OUTPUT);
  print_string_size(map->creator, 16, OUTPUT);
  fprintf(OUTPUT, "\n");
}
