#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osux_db.h"
#include "util/md5.h"

int main(void)
{
    struct osux_db *db;
    
    osux_db_load("./osu.db", &db);

    osux_db_update_stat(db);
    osux_db_print_stat(stdout, db);
    osux_db_dump(stdout, db);
    osux_db_free(db);
    
    return EXIT_SUCCESS;
}
