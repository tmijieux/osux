#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osux_db.h"

int main(int argc, char *argv[]) 
{
    osux_db *db;
    /* osux_db_build("/mnt/windata/Songs", &odb); */
    /* osux_db_write("osu.db", &odb); */
    /* osux_db_dump(stdout, &odb); */

    osux_db_load("osu.db", &db );
    osux_db_query_print(stdout, "select * from beatmap", db);
    
    return EXIT_SUCCESS;
}
