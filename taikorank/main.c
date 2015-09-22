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
#include "convert.h"
#include "print.h"

#include "compute.h"
#include "density.h"
#include "reading.h"

int compare(char* s, char* t, int length);
int check_file(char * file_name);
void main_one_file (char * file_name, int argc);

// ./taiko_ranking aa_maps/* 2> null | sort -n

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

int compare(char* s, char* t, int length)
{
  for (int i = 0; i < length; ++i)
    if (s[i] != t[i])
      return 0;
  return 1;
}

//--------------------------------------------------

int check_file(char * file_name)
{
  // cheking that it's a .osu file
  int length = strlen(file_name);
  if (!compare(".osu", &file_name[length-4], 5))
    {
      fprintf(OUTPUT_ERR, "%s: This isn't a .osu file, are you kidding me ?!\n", file_name);
      return 0;
    }
  // check that the file existence
  if (access(file_name, F_OK) == -1)
    {
      fprintf(OUTPUT_ERR, "%s: Please let me open your file :S\n", file_name);
      return 0;
    }
  return 1;
}

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

void main_one_file(char * file_name, int argc)
{
  // check and reading
  if (check_file(file_name) == 0)
    return;

  struct tr_map * map = convert(file_name);
  if (map == NULL)
    return;

  // compuatation
  compute_hand(map);
  compute_rest(map);
  compute_density(map);
  compute_reading(map);

  // printing
  if (argc == 2)
    {
      print_all_tr_object(map, FILTER_APPLY);
      print_map_star(map);
    }
  print_map_final(map);
  
  // free
  free(map->title);
  free(map->creator);
  free(map->diff);  
  free(map->object);
  free(map);
}

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

int main(int argc, char* argv[])
{
  // checking arguments
  if (argc >= 2)
    {
      for (int i = 1; i < argc; i++)
	main_one_file(argv[i], argc);
    }
  else
    fprintf(OUTPUT_ERR, "No osu file D:\n");

  return EXIT_SUCCESS;
}

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------
