#ifndef OSUXDB_H
#define OSUXDB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <beatmap/beatmap.h>

struct osux_db;
typedef struct osux_db osux_db;
typedef struct osux_beatmap osux_beatmap;

int osux_db_create(struct osux_db **db);
int osux_db_init(struct osux_db *db);

int osux_db_build(const char *song_path, osux_db **odb);
int osux_db_save(const char *filename, const osux_db *odb);
int osux_db_load(const char *filename, osux_db **odb);

int osux_db_print_stat(FILE *outfile, const struct osux_db *db);
int osux_db_update_stat(struct osux_db *db);

int osux_db_query_print(FILE *output, const char *query, const osux_db *db);

int osux_db_dump(FILE *outfile, const osux_db *odb);
int osux_db_free(osux_db *odb);
int osux_db_hash(osux_db *odb);

const char *osux_db_relative_path_by_hash(osux_db *odb, const char *hash);
osux_beatmap *osux_db_get_beatmap_by_hash(osux_db *odb, const char *hash);

#endif //OSUXDB_H


