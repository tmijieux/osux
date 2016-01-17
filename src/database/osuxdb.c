#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "util/list.h"
#include "util/split.h"
#include "util/md5.h"
#include "osuxdb.h"

static int parse_beatmap_rec(const char *name, int level, struct list *beatmaps)
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
            fprintf(stderr, "Entering directory %s\n", path);
            parse_beatmap_rec(path, level + 1, beatmaps);
            free(path);
        } else {
            if (!string_have_extension(path, ".osu")) {
                free(path);
                continue;
            }
            fprintf(stderr, "Found osu file %s\n", path);
            struct beatmap_info *bi = malloc(sizeof*bi);
            bi->osu_file_path = path;
            FILE *f = fopen(path, "r");
            if (NULL != f) {
                unsigned char *md5 = osux_md5_hash_file(f);
                fprintf(stderr, "osu file ok: %s\n", path);
                bi->md5_hash = osux_md5_string(md5);
                free(md5);
                fclose(f);
                list_append(beatmaps, bi);
            } else {
                fprintf(stderr, "osu file BUG: %s\n", path);
                free(path);
            }
        }
    } while ( (entry = readdir(dir)) != NULL );
    closedir(dir);

    return 0;
}

int osux_db_build(const char *directory_name, struct osudb *odb)
{
    assert( NULL != odb );
    struct list *beatmaps = list_new(0);
    parse_beatmap_rec(directory_name, 0, beatmaps);

    size_t n = list_size(beatmaps);
    struct beatmap_info *bis = malloc(sizeof*bis * n);
    for (unsigned int i = 1; i <= n; ++i) {
        struct beatmap_info *bm = list_get(beatmaps, i);
        bis[i-1] = *bm;
        free(bm);
    }
    list_free(beatmaps);

    odb->beatmaps_number = n;
    odb->beatmaps = bis;
    return 0;
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
