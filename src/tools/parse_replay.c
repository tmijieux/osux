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

int main(int argc, char *argv[])
{
    char *path = NULL;
    if (argc != 2)  {
        fprintf(stderr, "Usage: %s /path/to/replay.osr\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    osux_replay r;
    if (osux_replay_init(&r, argv[1]) < 0) {
        printf("Cannot parse replay %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    osux_replay_print(&r, stdout);

    osux_beatmap_db db;
    if (osux_beatmap_db_init(&db, "./osux.sqlite", ".", false) == 0) {
        path = osux_beatmap_db_get_path_by_hash(&db, r.beatmap_hash);
        if (path != NULL)
            printf("Replay map: %s\n", path);
        else
            printf("Replay map: Not found.\n");
        osux_beatmap_db_free(&db);
    }


    osux_beatmap bm;
    if (path != NULL) {
        if (osux_beatmap_init(&bm, path) < 0) {
            fprintf(stderr, "Cannot parse beatmap %s\n", path);
            exit(EXIT_FAILURE);
        }

        osux_hits hits;
        osux_hits_init(&hits, &bm, &r);
        osux_hits_free(&hits);

        osux_beatmap_free(&bm);
    }

    osux_replay_free(&r);
    return EXIT_SUCCESS;
}
