#ifndef OSUX_BEATMAP_DATABASE_H
#define OSUX_BEATMAP_DATABASE_H

#include "osux/database.h"

typedef struct osux_beatmap_database_ {
    osux_database base;
    char *song_dir;
    size_t song_dir_length;

    bool insert_prepared;
} osux_beatmap_db;


int osux_beatmap_db_init(osux_beatmap_db *db, char const *file_path,
                         char const *song_dir, bool populate);
int osux_beatmap_db_free(osux_beatmap_db *db);
char *osux_beatmap_db_get_beatmap_path_by_hash(osux_beatmap_db *db,
                                               char const *md5_hash);


#endif // OSUX_BEATMAP_DATABASE_H
