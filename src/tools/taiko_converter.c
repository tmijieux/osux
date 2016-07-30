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

#include "osux.h"

static void check_arg(int argc)
{
    if (argc < 2) {
	fprintf(stderr, "Usage:\ntaiko_converter path/to/input.osu\n");
	exit(EXIT_FAILURE);
    }
}

static osux_beatmap * load_beatmap(const char * path)
{
    osux_beatmap * bm = NULL;
    int res = osux_beatmap_open(path, &bm);
    if (res < 0) {
	fprintf(stderr, "Failed to load beatmap '%s'\n", path);
	return NULL;
    }
    return bm;
}


int main(int argc, char * argv[])
{
    check_arg(argc);
    for (int i = 1; i < argc; i++) {
	osux_beatmap * bm = load_beatmap(argv[i]);
	if (bm == NULL)
	    continue;
	
	int res_convert = osux_beatmap_taiko_autoconvert(bm);
	if (res_convert < 0) {
	    fprintf(stderr, "Failed to convert beatmap '%s'\n", argv[i]);
	    goto end;
	}

	bm->Version = xasprintf("%s%s", bm->Version, " convert");
	
	int res_save = osux_beatmap_save(NULL, bm);
	if (res_save < 0) {
	    fprintf(stderr, "Failed to save beatmap '%s'\n", argv[i]);
	    goto end;
	}
	    
	fprintf(stderr, "Successful output for beatmap '%s'\n", argv[i]);
    end:
	osux_beatmap_close(bm);
    }
    return EXIT_SUCCESS;
}
