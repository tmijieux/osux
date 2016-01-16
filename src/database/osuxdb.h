#ifndef OSUXDB_H
#define OSUXDB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct beatmap_info {
    char *osu_file_path;
    char *md5_hash;
};

struct osudb {
    uint32_t beatmaps_number;
    struct beatmap_info *beatmaps;
};

void osux_rebuild_db(const char *directory_name, struct osudb *odb);

#endif //OSUXDB_H
