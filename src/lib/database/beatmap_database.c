#include <glib.h>
#include "osux/beatmap.h"
#include "osux/error.h"
#include "osux/string.h"
#include "osux/beatmap_database.h"
#include "beatmap_db.sql.h"

static int init_schema(osux_beatmap_db *db)
{
    return osux_database_exec_query(&db->base, (char*) _beatmap_db_data, NULL);
}

static bool beatmap_table_is_present(osux_beatmap_db *db)
{
    osux_list *result = osux_list_new(LI_FREE, osux_hashtable_delete);
    int err = osux_database_exec_query(
        &db->base, "SELECT name FROM sqlite_master "
        "WHERE type='table' AND name='beatmap'", result);
    if (err < 0)
        return false;

    bool present = osux_list_size(result) == 1;
    osux_list_free(result);
    return present;
}

#define DB_BIND_INT(db, name, text)                                     \
    if ((ret = osux_database_bind_int(&(db)->base, (":"name), (text))) < 0) \
        return ret

#define DB_BIND_DOUBLE(db, name, text)                                  \
    if ((ret = osux_database_bind_double(&(db)->base, (":"name), (text))) < 0) \
        return ret

#define DB_BIND_TEXT(db, name, text)                                    \
    if ((ret = osux_database_bind_string(&(db)->base, (":"name), (text))) < 0) \
        return ret

static int beatmap_insert(osux_beatmap_db *db, osux_beatmap *bm)
{
    int ret;
    if (!db->insert_prepared) {
        ret = osux_database_prepare_query(
            &db->base, "INSERT INTO beatmap "
            "(osu_beatmap_id, game_mode, audio_filename, diff_name,"
            "md5_hash, osu_filename, file_path,"
            "circles, sliders, spinners, last_modification, last_checked,"
            "approach_rate, circle_size, hp_drain, overall_diff,"
            "slider_velocity, stack_leniency, drain_time, total_time,"
            "preview_time, bpm_avg, bpm_max, bpm_min, local_offset,"
            "online_offset, already_played, last_played, ignore_hitsound,"
            "ignore_skin, disable_sb, disable_video, visual_override,"
            "mania_scroll_speed)"
            " VALUES"
            "(:osu_beatmap_id, :game_mode, :audio_filename, :diff_name,"
            ":md5_hash, :osu_filename, :file_path,"
            ":circles, :sliders, :spinners, :last_modification, :last_checked,"
            ":approach_rate, :circle_size, :hp_drain, :overall_diff,"
            ":slider_velocity, :stack_leniency, :drain_time, :total_time,"
            ":preview_time, :bpm_avg, :bpm_max, :bpm_min, :local_offset,"
            ":online_offset, :already_played, :last_played, :ignore_hitsound,"
            ":ignore_skin, :disable_sb, :disable_video, :visual_override,"
            ":mania_scroll_speed)");
        if (ret < 0)
            return ret;
        db->insert_prepared = true;
    }

    DB_BIND_INT(db, "osu_beatmap_id", bm->BeatmapID);
    DB_BIND_INT(db, "game_mode", bm->Mode);
    DB_BIND_TEXT(db, "audio_filename", bm->AudioFilename);
    DB_BIND_TEXT(db, "md5_hash", bm->md5_hash);
    DB_BIND_TEXT(db, "osu_filename", bm->osu_filename);
    DB_BIND_TEXT(db, "file_path", bm->file_path);
    DB_BIND_TEXT(db, "diff_name", bm->Version);
    DB_BIND_INT(db, "circles", bm->circles);
    DB_BIND_INT(db, "sliders", bm->sliders);
    DB_BIND_INT(db, "spinners", bm->spinners);
    DB_BIND_INT(db, "last_modification", bm->last_modification);
    DB_BIND_INT(db, "last_checked", bm->last_checked);
    DB_BIND_DOUBLE(db, "approach_rate", bm->ApproachRate);
    DB_BIND_DOUBLE(db, "circle_size", bm->CircleSize);
    DB_BIND_DOUBLE(db, "hp_drain", bm->HPDrainRate);
    DB_BIND_DOUBLE(db, "overall_diff", bm->OverallDifficulty);
    DB_BIND_DOUBLE(db, "slider_velocity", bm->SliderMultiplier);
    DB_BIND_DOUBLE(db, "stack_leniency", bm->StackLeniency);
    DB_BIND_INT(db, "drain_time", bm->drain_time);
    DB_BIND_INT(db, "total_time", bm->total_time);
    DB_BIND_INT(db, "preview_time", bm->PreviewTime);
    DB_BIND_INT(db, "bpm_avg", bm->bpm_avg);
    DB_BIND_INT(db, "bpm_min", bm->bpm_min);
    DB_BIND_INT(db, "bpm_max", bm->bpm_max);
    DB_BIND_INT(db, "local_offset", bm->local_offset);
    DB_BIND_INT(db, "online_offset", bm->online_offset);
    DB_BIND_INT(db, "already_played", bm->already_played);
    DB_BIND_INT(db, "last_played", bm->last_played);
    DB_BIND_INT(db, "ignore_hitsound", bm->ignore_hitsound);
    DB_BIND_INT(db, "ignore_skin", bm->ignore_skin);
    DB_BIND_INT(db, "disable_sb", bm->disable_sb);
    DB_BIND_INT(db, "disable_video", bm->disable_video);
    DB_BIND_INT(db, "visual_override", bm->visual_override);
    DB_BIND_INT(db, "mania_scroll_speed", bm->mania_scroll_speed);

    return osux_database_exec_prepared_query(&db->base, NULL);
}

static int load_beatmap_from_disk(
    osux_beatmap_db *db, const char *filepath)
{
    int err = 0;
    osux_beatmap beatmap;

    if ((err = osux_beatmap_init(&beatmap, filepath)) < 0) {
        osux_error("Cannot load beatmap\nfilename:%s\nerror type: %s\n\n",
                   filepath, osux_errmsg(err));
        return err;
    }
    if ((err = beatmap_insert(db, &beatmap) < 0))
        fprintf(stderr, "inserting beatmap '%s' failed\n", beatmap.file_path);
    else {
        ++ db->parsed_beatmap_count;
        if (beatmap.BeatmapID)
            printf("%% %ld %s [%s]\n",
                   beatmap.BeatmapID, beatmap.Title, beatmap.Version);
        else
            printf("%% %s [%s]\n", beatmap.Title, beatmap.Version);
    }
    osux_beatmap_free(&beatmap);
    return err;
}

static gboolean parse_beatmap_directory(osux_beatmap_db *db, char const *path)
{
    GDir *dir = NULL;
    const gchar *entry_name;

    assert( db != NULL );

    if ((dir = g_dir_open(path, 0, NULL)) == NULL)
        return -1;
    if ((entry_name = g_dir_read_name(dir)) == NULL)
        return 0;

    do {
        char *file_entry = g_strdup_printf("%s/%s", path, entry_name);
        if (g_file_test(file_entry, G_FILE_TEST_IS_DIR)) {
            // with GLib "." and ".." directories are ommited on Unix platforms
            parse_beatmap_directory(db, file_entry);
            g_free(file_entry);
        } else {
            if (!string_have_extension(file_entry, ".osu")) {
                g_free(file_entry); // ignore non-osu file
                continue;
            }
            load_beatmap_from_disk(db, file_entry);
            g_free(file_entry);
        }
    } while ((entry_name = g_dir_read_name(dir)) != NULL);
    g_dir_close(dir);
    return 0;
}

char *osux_beatmap_db_get_path_by_hash(
    osux_beatmap_db *db, char const *md5_hash)
{
    osux_list *query_result = osux_list_new(LI_FREE, osux_hashtable_delete);
    db->insert_prepared = false;
    osux_database_prepare_query(&db->base, "SELECT file_path FROM beatmap "
                                "WHERE md5_hash = :hash");
    osux_database_bind_string(&db->base, ":hash", md5_hash);
    osux_database_exec_prepared_query(&db->base, query_result);

    char *res = NULL;
    if (osux_list_size(query_result) != 0) {
        osux_hashtable *dict = (osux_hashtable*) osux_list_get(query_result, 1);
        if (osux_hashtable_lookup(dict, "file_path", &res) < 0)
            res = NULL;
        else
            res = g_strdup(res);
    }
    osux_list_free(query_result);
    return res;
}

int osux_beatmap_db_init(
    osux_beatmap_db *db, char const *file_path, char const *song_dir, bool populate)
{
    int err;
    memset(db, 0, sizeof*db);
    if ((err = osux_database_init(&db->base, file_path)) < 0)
        return err;

    db->song_dir = g_strdup(song_dir);
    db->song_dir_length = strlen(song_dir);
    db->insert_prepared = false;

    if (populate || !beatmap_table_is_present(db)) {
        if ((err = init_schema(db)) < 0)
            return err;
        if (populate) {
            parse_beatmap_directory(db, song_dir);
            printf("parsed %lu beatmap%s.\n",
                   db->parsed_beatmap_count, db->parsed_beatmap_count ? "s":"");
        }
    }
    return 0;
}

int osux_beatmap_db_free(osux_beatmap_db *db)
{
    g_free(db->song_dir);
    osux_database_free(&db->base);
    memset(db, 0, sizeof*db);
    return 0;
}

int osux_beatmap_db_dump(osux_beatmap_db *db, FILE *out)
{
    return osux_database_print_query(&db->base, "SELECT * FROM beatmap", out);
}
