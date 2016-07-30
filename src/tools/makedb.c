#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osux.h"

int main(void)
{
    osux_db *db;
	const char *songDirPath = "E:/Songs2/";
    if (osux_db_build(songDirPath, &db) < 0) {
		fprintf(stderr, "Cannot build database from directory '%s'", songDirPath);
		exit(EXIT_FAILURE);
	}
    /* osux_db_load("osu.db", &db); */
    /* osux_db_init(db); */
    /* osux_db_query_print(stdout, "select * from beatmap", db); */
    
    osux_db_save("osu.db", db);
    osux_db_free(db);

    return EXIT_SUCCESS;
}
