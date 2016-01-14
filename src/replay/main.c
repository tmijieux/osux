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

#include <stdio.h>
#include <stdlib.h>

#include "replay.h"

int main(int argc, char *argv[])
{
    if (argc != 2)  {
	fprintf(stderr, "Usage: %s replay.osr\n", argv[0]);
	exit(EXIT_FAILURE);
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
	perror(argv[1]);
	exit(EXIT_FAILURE);
    }

    struct replay *r = replay_parse(f);
    fclose(f);
	
    replay_print(stdout, r);
    replay_free(r);
    
    return EXIT_SUCCESS;
}
