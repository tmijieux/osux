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

static void check_arg(int argc, char **argv)
{
    if (argc < 2) {
	fprintf(stderr, "Usage:\n\t%s /path/to/input.osu\n", argv[0]);
	exit(EXIT_FAILURE);
    }
}

static bool load_beatmap(osux_beatmap *bm, char const *path)
{
    if (osux_beatmap_init(bm, path) < 0) {
	fprintf(stderr, "Failed to load beatmap '%s'\n", path);
        return false;
    }
    return true;
}


int main(int argc, char * argv[])
{
    check_arg(argc, argv);

    for (int i = 1; i < argc; i++) {
        osux_beatmap bm;
        
	if (!load_beatmap(&bm, argv[i]))
	    continue;
	
	if (osux_beatmap_taiko_autoconvert(&bm) < 0) {
	    fprintf(stderr, "Failed to convert beatmap '%s'\n", argv[i]);
	    goto end;
	}
        char *new_diff_name = g_strdup_printf("%s convert", bm.Version);
	g_free(bm.Version);
        bm.Version = new_diff_name;
	
	if (osux_beatmap_save_full(&bm, ".", NULL, true) < 0) {
	    fprintf(stderr, "Failed to save beatmap '%s'\n", argv[i]);
	    goto end;
	}
	    
	fprintf(stderr, "Successful output for beatmap '%s'\n", argv[i]);
    end:
	osux_beatmap_free(&bm);
    }
    return EXIT_SUCCESS;
}
