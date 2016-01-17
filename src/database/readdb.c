#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osuxdb.h"
#include "util/md5.h"

int main(int argc, char *argv[]) 
{

    struct osudb odb;
    osux_db_read("./osu.db", &odb);
    osux_db_dump(stdout, &odb);

    osux_db_free(&odb);
    
    return EXIT_SUCCESS;
}
