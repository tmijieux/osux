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

#include "struct.h"
#include "print.h"

//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------

void print_all_tr_object (struct tr_map * map, int filter)
{
  if ((filter & FILTER_BASIC) != 0)
    fprintf(OUTPUT, "offset\tend\trest\ttype\tbpm app\t");
  if ((filter & FILTER_ADDITIONNAL) != 0)
    fprintf(OUTPUT, "l hand\tr hand\trest\t");
  if ((filter & FILTER_DENSITY) != 0)
    fprintf(OUTPUT, "dnst rw\tdnst cl\tdnst*\t");
  if ((filter & FILTER_READING) != 0)
    fprintf(OUTPUT, "app\tend app\tdis\tend dis\tsperpos\thidden\thide\tspeed\tspd chg\tread*\t");
  if ((filter & FILTER_PATTERN) != 0)
    fprintf(OUTPUT, "\t");
  if ((filter & FILTER_ACCURACY) != 0)
    fprintf(OUTPUT, "\t");
  
  fprintf(OUTPUT, "\n");
  
  for (int i = 0; i < map->nb_object; ++i)
    {
      print_one_tr_object(&map->object[i], filter);
    }
}

//-------------------------------------------------

void print_one_tr_object (struct tr_object * obj, int filter)
{
  if ((filter & FILTER_BASIC) != 0)
    fprintf(OUTPUT, "%d\t%d\t%d\t%c\t%.3g\t",
	    obj->offset,
	    obj->end_offset,
	    obj->rest,
	    obj->type,
	    obj->bpm_app);
  if ((filter & FILTER_ADDITIONNAL) != 0)
    fprintf(OUTPUT, "%d\t%d\t",
	    obj->l_hand,
	    obj->r_hand);
  if ((filter & FILTER_DENSITY) != 0)
    fprintf(OUTPUT, "%g\t%g\t%g\t",
	    obj->density_raw,
	    obj->density_color,
	    obj->density_star);
  if ((filter & FILTER_READING) != 0)
    fprintf(OUTPUT, "%d\t%d\t%d\t%d\t%g\t%.3g\t%.3g\t%g\t%.2g\t%g\t",
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
    fprintf(OUTPUT, "\t");
  if ((filter & FILTER_ACCURACY) != 0)
    fprintf(OUTPUT, "\t");
  if ((filter & FILTER_STAR) != 0)
    fprintf(OUTPUT, "%g\t%g\t",
	    obj->density_star,
  	    obj->reading_star);
  
  fprintf(OUTPUT, "\n");
}

//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------

void print_map_star (struct tr_map * map)
{
  fprintf(OUTPUT, "Density star: \t%.15g\n",
	  map->density_star);
  fprintf(OUTPUT, "Reading star: \t%.15g\n",
	  map->reading_star);
  /*
  fprintf(OUTPUT, "Pattern star: \t%g\n",
	  map->pattern_star);
  
  fprintf(OUTPUT, "Accuracy star: \t%g\n",
  map->accuracy_star);
  */
}

//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------

void print_string_size (char * s, int max)
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
  fprintf(OUTPUT, "%s", s);
  length = max - length - 1;
  for (int i = 0; i < length; i++)
      fprintf(OUTPUT, " ");
  fprintf(OUTPUT, "\t");
}

//-------------------------------------------------

void print_map_final (struct tr_map * map)
{
  fprintf(OUTPUT, "%g      \t", map->reading_star);
  fprintf(OUTPUT, "%g      \t", map->density_star);
  print_string_size(map->diff,    32);
  print_string_size(map->title,   32);
  print_string_size(map->creator, 32);
  fprintf(OUTPUT, "\n");
}
