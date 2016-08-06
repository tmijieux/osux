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
#include <glib.h>
#include <glib/gstdio.h>

#include "osux.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /path/to/beatmap.osu\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int err;
    osux_beatmap bm;
    if ((err = osux_beatmap_init(&bm, argv[1])) < 0) {
        fprintf(stderr, "Cannot parse beatmap '%s': %s\n",
                argv[1], osux_errmsg(err));
        return EXIT_FAILURE;
    }

    osux_beatmap_print(&bm, stdout);
    osux_beatmap_free(&bm);

    return EXIT_SUCCESS;
}
