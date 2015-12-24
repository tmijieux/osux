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
#include "taiko_ranking_score.h"
#include "mods.h"
#include "print.h"

int main(int argc, char* argv[])
{
  // checking arguments
  if (argc >= 2)
    {      
      for (int i = 1; i < argc; i++)
	{
	  struct tr_map * map = trm_new(argv[i]);
	  if (map == NULL)
	    continue;
	  
	  //trm_main(map, MODS_NONE);
	  trs_main(map, MODS_NONE);
	  
	  //trm_main(map, MODS_DT);
	  //trm_main(map, MODS_HT);
	  //trm_main(map, MODS_HD);
	  //trm_main(map, MODS_FL);
	  //trm_main(map, MODS_HR);
	  //trm_main(map, MODS_HD);
	  //trm_main(map, MODS_HD | MODS_FL);
	  
	  trm_free(map);	    
	}
    }
  else
    tr_error("No osu file D:");

  return EXIT_SUCCESS;
}
