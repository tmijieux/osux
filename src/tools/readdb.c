#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osux.h"

int main(int argc, char **argv)
{
    osux_beatmap_db db;
    const char *db_path = "./osu.db";
    
    if (argc > 1)
        db_path = argv[1];
    
    if (osux_beatmap_db_init(&db, db_path, ".", false) < 0) {
        fprintf(stderr, "Cannot load databse '%s'", db_path);
        exit(EXIT_FAILURE);
    }

    osux_beatmap_db_dump(&db, stdout);
    osux_beatmap_db_free(&db);
    
    return EXIT_SUCCESS;
}
