#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "util/list.h"
#include "util/md5.h"
#include "osuxdb.h"

static struct list*
parse_beatmap_rec(const char *name, int level, struct list *beatmaps)
{
    DIR *dir;
    struct dirent *entry;

    assert( beatmaps != NULL );
    
    if (!(dir = opendir(name)))
        return beatmaps;
    if (!(entry = readdir(dir)))
        return beatmaps;

    do {
        char *path;
        asprintf(&path, "%s/%s", name, entry->d_name);        
        if (DT_DIR == entry->d_type) {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
                continue;
            parse_beatmap_rec(path, level + 1, beatmaps);
            free(path);
        } else {
            struct beatmap_info *bi = malloc(sizeof*bi);
            bi->osu_file_path = path;
            FILE *f = fopen(path, "r");
            if (NULL != f) {
                bi->md5_hash = (char*) osux_md5_hash_file(f);
                fclose(f);
                list_append(beatmaps, bi);
            } else {
                free(path);
            }
        }
    } while ( (entry = readdir(dir)) != NULL );
    closedir(dir);

    return beatmaps;
}

void osux_rebuild_db(const char *directory_name, struct osudb *odb)
{
    assert( NULL != odb );
    struct list *beatmaps;
    beatmaps = parse_beatmap_rec(directory_name, 0, list_new(0));

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
}
