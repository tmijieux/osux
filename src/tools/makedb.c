#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osux.h"

int main(int argc, char **argv)
{
    const char *songDirPath = "/mnt/windata/Songs2/";
    if (argc > 2)
        songDirPath = argv[1];
    
    osux_beatmap_db db;
    printf("building from song directory '%s'\n", songDirPath);
    if (osux_beatmap_db_init(&db, "./osu.db", songDirPath, true) < 0) {
        fprintf(stderr, "Cannot build database from directory '%s'", songDirPath);
        exit(EXIT_FAILURE);
    }
    osux_beatmap_db_free(&db);
    return EXIT_SUCCESS;
}
