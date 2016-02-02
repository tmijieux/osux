#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osux_db.h"

int main(int argc, char *argv[])
{
    osux_db *db;
    /* osux_db_build("/mnt/windata/Songs", &odb); */
    /* osux_db_create(&db); */

    osux_db_load("osu.db", &db );
    osux_db_init(db);

    osux_db_query_print(stdout, "select * from beatmap", db);
    /* osux_db_save("osu.db", db); */
    osux_db_free(db);

    return EXIT_SUCCESS;
}
