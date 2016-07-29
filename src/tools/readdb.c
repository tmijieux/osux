#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osux.h"

int main(void)
{
    struct osux_db *db;
    
    if (osux_db_load("./osu.db", &db) < 0) {
        exit(EXIT_FAILURE);
    }

    osux_db_update_stat(db);
    osux_db_print_stat(stdout, db);
    osux_db_dump(stdout, db);
    osux_db_free(db);
    
    return EXIT_SUCCESS;
}
