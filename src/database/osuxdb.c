#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "beatmap/parser/parser.h"
#include "util/list.h"
#include "util/hash_table.h"
#include "util/split.h"
#include "util/md5.h"
#include "util/data.h"
#include "osuxdb.h"

static int 
load_beatmap(struct osudb *odb, const char *filename, int base_path_length)
{
    char *errmsg, *md5_hash;
    unsigned char *md5;
    FILE *f;

    f = fopen(path, "r");
    if (NULL != f) {
        printf("db_parse: osu file: %s\n", path);
    } else {
        osux_error("osu file BUG: %s\n", path);
        return -1;
    }

    errmsg = NULL;
    md5 = osux_md5_hash_file(f);
    sqlite3_exec(odb->db, "INSERT INTO", NULL, NULL, &errmsg);
    sqlite3_free(errmsg);
    md5_hash = osux_md5_string(md5);
    free(md5);
    
    fclose(f);
    return 0;
}


static int
parse_beatmap_directory_rec(const char *name, struct osudb *odb,
                            int base_path_length, int level)

{
    DIR *dir;
    struct dirent *entry;

    assert( beatmaps != NULL );


    if (!(dir = opendir(name)))
        return -1;
    if (!(entry = readdir(dir)))
        return -1;

    do {
        char *path;
        asprintf(&path, "%s/%s", name, entry->d_name);
        if (DT_DIR == entry->d_type) {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0) {
                free(path);
                continue;
            }
            parse_beatmap_rec(path, level + 1, beatmaps, base_path_length);
            free(path);
        } else {
            if (!string_have_extension(path, ".osu")) {
                free(path);
                continue;
            }
            load_beatmap(odb, path, base_path_length);
            free(path);
        }
    } while ((entry = readdir(dir)) != NULL);
    closedir(dir);

    return 0;
}


int osux_db_init(struct osudb *odb)
{
    assert( NULL != odb );
    memset(odb, 0, sizeof*odb);
    
    ret = sqlite3_open(":memory:", &odb->db);
    if (ret) {
        fprintf(stderr, "%s:%s:%s:Can't open database: %s\n",
                __PRETTY_FUNCTION__, __FILE__, __LINE__,
                sqlite3_errmsg(odb->db));
        sqlite3_close(db);
        return -1;
    }

    char *errmsg = NULL;
    sqlite3_exec(odb->db, "create table",
                 NULL, NULL, &errmsg);
}

int osux_db_build(const char *directory_name, struct osudb *odb)
{



    return parse_beatmap_directory_rec(directory_name, odb,
                                       strlen(directory_name), 0);
}

int osux_db_write(const char *filename, const struct osudb *odb)
{
    FILE *f;
    assert( NULL != odb );

    if ((f = fopen(filename, "w+")) == NULL)
        return -1;
    fwrite(&odb->beatmaps_number, sizeof odb->beatmaps_number, 1, f);
    for (uint32_t i = 0; i < odb->beatmaps_number; ++i) {
        write_string_ULEB128(odb->beatmaps[i].osu_file_path, f);
        write_string_ULEB128(odb->beatmaps[i].md5_hash, f);
    }
    fclose(f);
    return 0;
}

int osux_db_read(const char *filename, struct osudb *odb)
{
    FILE *f;
    assert( NULL != odb );
    memset(odb, 0, sizeof*odb);

    if ((f = fopen(filename, "r")) == NULL) {
        odb->beatmaps_number = 0;
        odb->beatmaps = NULL;
        return -1;
    }
    fread(&odb->beatmaps_number, sizeof odb->beatmaps_number, 1, f);
    odb->beatmaps = malloc(sizeof*odb->beatmaps * odb->beatmaps_number);

    for (uint32_t i = 0; i < odb->beatmaps_number; ++i) {
        read_string_ULEB128(&odb->beatmaps[i].osu_file_path, f);
        read_string_ULEB128(&odb->beatmaps[i].md5_hash, f);
    }
    fclose(f);
    return 0;
}


void osux_db_free(struct osudb *odb)
{
    if (NULL != odb) {
        for (uint32_t i = 0; i < odb->beatmaps_number; ++i) {
            free(odb->beatmaps[i].osu_file_path);
            free(odb->beatmaps[i].md5_hash);
        }
        free(odb->beatmaps);
        if (odb->db_hashed) {
            ht_free(odb->hashmap);
        }
    }
}

void osux_db_dump(FILE *outfile, const struct osudb *odb)
{
    fprintf(outfile, "Number of maps: %d\n", odb->beatmaps_number);
    for (uint32_t i = 0; i < odb->beatmaps_number; ++i) {
        fprintf(outfile, "Beatmap #%d:\nfile_path: %s\n"
                "beatmap md5 hash: %s\n\n",
                i, odb->beatmaps[i].osu_file_path,
                odb->beatmaps[i].md5_hash);
    }
}

void osux_db_hash(struct osudb *odb)
{
    struct hash_table *hashmap = ht_create(odb->beatmaps_number, NULL);

    for (unsigned i = 0; i < odb->beatmaps_number; ++i)
        ht_add_entry(hashmap,
                     odb->beatmaps[i].md5_hash,
                     odb->beatmaps[i].osu_file_path);

    odb->hashmap = hashmap;
    odb->db_hashed = true;
}

const char*
osux_db_relpath_by_hash(struct osudb *odb, const char *hash)
{
    char *ret = NULL;
    ht_get_entry(odb->hashmap, hash, &ret);
    return ret;
}

struct map*
osux_db_get_beatmap_by_hash(struct osudb *odb, const char *hash)
{
    char *path = osux_prefix_path(osux_get_song_path(),
                                  osux_db_relpath_by_hash(odb, hash));
    struct map * m =  osux_parse_beatmap(path);
    free(path);

    return m;
}
