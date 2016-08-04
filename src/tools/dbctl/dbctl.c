#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "osux.h"
#include "cmdline.h"


static void list_database(GKeyFile *key_file)
{
    gsize len;
    char **keys = g_key_file_get_groups(key_file, &len);
    for (unsigned i = 0; i < len; ++i) {
        char *songDir = g_key_file_get_string(key_file, keys[i], "songDir", NULL);
        if (songDir != NULL) {
            printf(" - %s: %s\n", keys[i], songDir);
            g_free(songDir);
        } else
            printf(" - %s\n", keys[i]);
    }
    g_strfreev(keys);
}

int main(int argc, char *argv[])
{
    struct gengetopt_args_info info;
    GKeyFile *key_file;
    
    if (cmdline_parser(argc, argv, &info) != 0) {
        fprintf(stderr, "error parsing command line arguments\n");
        exit(EXIT_FAILURE);
    }

    key_file = g_key_file_new();    
    g_key_file_load_from_file(
        key_file, info.config_arg, G_KEY_FILE_KEEP_COMMENTS, NULL);

    if (info.list_given) {
        list_database(key_file);
        return 0;
    }

    char const *database = info.database_arg;
    printf("Using database: '%s'\n", database);
    
    // get song directory:
    char *song_directory = info.song_arg; // by default use default value
    if (g_key_file_has_group(key_file, database)) {
        if (g_key_file_has_key(key_file, database, "songDir", NULL)) {
            song_directory = g_key_file_get_string(key_file, database,"songDir", NULL);
            // if a value is present in config, use this value ...
        }
    }
    if (info.song_given) {
        // ... unless user explicity specified a value on the command line
        if (song_directory != NULL)
                g_free(song_directory);
        song_directory = info.song_arg;
    }
    printf("Using song directory: '%s'\n", song_directory);

    if (info.populate_given + info.show_given + info.hash_given >= 2) {
        fprintf(stderr, "Cannot combine more than one of -p, -t or -h\n");
        return EXIT_FAILURE;
    }

    osux_beatmap_db db;
    osux_beatmap_db_init(&db, database, song_directory, info.populate_given);
    
    if (info.hash_given) {
        char *path = osux_beatmap_db_get_path_by_hash(&db, info.hash_arg);
        if (path != NULL)
            printf("%s\n", path);
        else
            fprintf(stderr, "%s: No match.\n", info.hash_arg);
    }
        
    if (info.show_given) {
        osux_beatmap_db_dump(&db, stdout);
    }

    g_key_file_set_string(key_file, database, "songDir", song_directory);
    g_key_file_save_to_file(key_file, info.config_arg, NULL);

    cmdline_parser_free(&info);

    return EXIT_SUCCESS;
}
