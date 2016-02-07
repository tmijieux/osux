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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "taiko_ranking_map.h"
#include "taiko_ranking_score.h"
#include "print.h"
#include "config.h"
#include "options.h"

int main(int argc, char* argv[])
{
  //set_app_dir(argv[0]);
  int nb_map = 0;
  char ** arg_map = malloc(sizeof(char*) * argc-1);
  for(int i = 1; i < argc; i++)
    {
      if(argv[i][0] == OPTIONS_PREFIX)
	{
	  i += options_set(argc - i, (const char **) &argv[i]);
	}
      else
	{
	  arg_map[nb_map] = argv[i];
	  nb_map++;
	}
    }
  
  // checking arguments
  if(nb_map > 0)
    { 
      void (* tr_main)(const struct tr_map *, int);
      if(OPT_SCORE)
	tr_main = trs_main;
      else
	tr_main = trm_main;
      
      for(int i = 0; i < nb_map; i++)
	{
	  struct tr_map * map = trm_new(arg_map[i]);
	  if(map == NULL)
	    continue;
	  
	  tr_main(map, OPT_MODS);
	  trm_free(map);
	}
    }
  else
    tr_error("No osu file D:");

  free(arg_map);
  return EXIT_SUCCESS;
}
