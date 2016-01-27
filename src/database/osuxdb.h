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
    
    struct hash_table *hashmap;
    bool db_hashed;
};

int osux_db_build(const char *directory_name, struct osudb *odb);
int osux_db_write(const char *filename, const struct osudb *odb);
int osux_db_read(const char *filename, struct osudb *odb);
void osux_db_dump(FILE *outfile, const struct osudb *odb);
void osux_db_free(struct osudb *odb);
void osux_db_hash(struct osudb *odb);
const char* osux_db_relpath_by_hash(struct osudb *odb, const char *hash);
struct map* osux_db_get_beatmap_by_hash(struct osudb *odb, const char *hash);

#endif //OSUXDB_H
