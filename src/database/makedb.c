#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osuxdb.h"
#include "util/md5.h"

int main(int argc, char *argv[]) 
{
    struct osudb odb;
    osux_db_build("/mnt/windata/Songs", &odb);
    osux_db_write("osu.db", &odb);
    osux_db_dump(stdout, &odb);
    
    return EXIT_SUCCESS;
}
