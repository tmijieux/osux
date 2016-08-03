#define _GNU_SOURCE

#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sqlite3.h>

#include "osux/parser.h"
#include "osux/list.h"
#include "osux/hash_table.h"
#include "osux/split.h"
#include "osux/md5.h"
#include "osux/data.h"
#include "osux/error.h"
#include "osux/string2.h"
#include "osux/osux_db.h"

struct osux_db_stat {
    uint32_t beatmap_count;
    uint32_t beatmap_set_count;
    time_t last_stat_time;
};

struct osux_db {
    struct osux_db_stat db_stat;
    sqlite3 *sqlite_db;
};

static int db_query(
    sqlite3 *db, void *callback, void *context, const char *format, ...)
{
    int ret;
    char *errmsg, *query;
    va_list ap;

    va_start(ap, format);
    query = xvasprintf(format, ap);
    ret = sqlite3_exec(db, query, callback, context, &errmsg);
    va_end(ap);

    if (NULL != errmsg) {
        osux_error("%s\nquery: %s\n", errmsg, query);
        sqlite3_free(errmsg);
        free(query);
        exit(EXIT_FAILURE);
    }
    free(query);
    return ret;
}

static int db_callback_get_uint32(
    void *context__,
    int count,
    char **column_text,
    char **column_name )
{
    (void) count;
    (void) column_name;

    uint32_t *value_ptr = context__;
    *value_ptr = atoi(column_text[0]);
    return 0;
}

int osux_db_update_stat(struct osux_db *db)
{
    db_query(
        db->sqlite_db, db_callback_get_uint32,
        &db->db_stat.beatmap_count, "select count(*) from beatmap;");
    db_query(
        db->sqlite_db, db_callback_get_uint32,
        &db->db_stat.beatmap_set_count, "select count(*) from beatmap_set;");
    db->db_stat.last_stat_time = time(NULL);
    return 0;
}


#define DATE(X) (get_time((X)))
#define HT_GET(X, WHAT)    WHAT(ht_get_(ht, (X)))

static inline gint64 get_time(const gchar *date_string)
{
    GTimeVal time;
    if (!g_time_val_from_iso8601(date_string, &time)) {
        fprintf(stderr, "error parsing date\n");
    }

    GDateTime *dateTime = g_date_time_new_from_timeval_local(&time);
    g_assert(dateTime != NULL);
    gint64 unixTime = g_date_time_to_unix(dateTime);
    g_date_time_unref(dateTime);
    return unixTime;
}

static inline char *ht_get_(osux_hashtable *ht, const char *key)
{
    char *_tmp;
    osux_hashtable_lookup(ht, key, &_tmp);
    return _tmp;
}

#define FLOAT(X) atof((X))
#define INT(X) atoi((X));
#define STRING(X) strdup((X));

static int beatmap_db_get_callback(
    void *context__, int count, char **column_text, char **column_name)
{
    struct osux_list *bm_list = context__;
    struct osux_beatmap *bm = calloc(sizeof*bm, 1);
    osux_hashtable *ht = osux_hashtable_new(0);

    osux_list_append(bm_list, bm);
    for (int i = 0; i < count; ++i)
        osux_hashtable_insert(ht, column_name[i], column_text[i]);

    bm->beatmap_id = HT_GET("beatmap_id", INT);
    bm->BeatmapID = HT_GET("osu_beatmap_id", INT);
    bm->BeatmapSetID = HT_GET("osu_beatmap_id", INT);
    bm->osu_filename = HT_GET("osu_filename", STRING);
    bm->md5_hash = HT_GET("md5_hash", STRING);
    bm->Mode = HT_GET("game_mode", INT);
    bm->AudioFilename = HT_GET("audio_filename", STRING);
    bm->Version = HT_GET("diff_name", STRING);
    bm->circles = HT_GET("circles", INT);
    bm->sliders = HT_GET("sliders", INT);
    bm->spinners = HT_GET("spinners", INT);
    bm->last_modification = HT_GET("last_modification", DATE);
    bm->last_checked = HT_GET("last_checked", DATE);
    bm->ApproachRate = HT_GET("approach_rate", FLOAT);
    bm->CircleSize = HT_GET("circle_size", FLOAT);
    bm->HPDrainRate = HT_GET("hp_drain", FLOAT);
    bm->OverallDifficulty = HT_GET("overall_diff", FLOAT);
    bm->SliderMultiplier = HT_GET("slider_velocity", FLOAT);
    bm->StackLeniency = HT_GET("stack_leniency", FLOAT);
    bm->drain_time = HT_GET("drain_time", INT);
    bm->total_time = HT_GET("total_time", INT);
    bm->PreviewTime = HT_GET("preview_time", INT);
    bm->bpm_avg = HT_GET("bpm_avg", INT);
    bm->bpm_max = HT_GET("bpm_max", INT);
    bm->bpm_min = HT_GET("bpm_min", INT);
    bm->local_offset = HT_GET("local_offset", INT);
    bm->online_offset = HT_GET("online_offset", INT);
    bm->already_played = HT_GET("already_played", INT);
    bm->last_played = HT_GET("last_played", DATE);
    bm->ignore_hitsound = HT_GET("ignore_hitsound", INT);
    bm->ignore_skin = HT_GET("ignore_skin", INT);
    bm->disable_sb = HT_GET("disable_sb", INT);
    bm->disable_video = HT_GET("disable_video", INT);
    bm->visual_override = HT_GET("visual_override", INT);
    bm->mania_scroll_speed  = HT_GET("mania_scroll_speed", INT);

    osux_hashtable_delete(ht);
    return 0;
}

static int fill_dict_list(
        void *user_data, int count, char **column_text, char **column_name)
{
    osux_list *query_result = (osux_list*) user_data;
    osux_hashtable *dict = osux_hashtable_new(0);
    
    for (int i = 0; i < count; ++i)
        osux_hashtable_insert(dict, column_name[i], column_text[i]);

    osux_list_append(query_result, dict);
    return 0;
}
    
#define DB_BIND_TEXT(STMT, NAME, TEXT)                                  \
    if (sqlite3_bind_text(                                              \
        STMT,                                                           \
        sqlite3_bind_parameter_index(STMT, ":"NAME),                    \
        TEXT,                                                           \
        -1,                                                             \
        NULL                                                            \
    ) != SQLITE_OK) {osux_error("error binding text\n");exit(1);}

#define DB_BIND_INT(STMT, NAME, TEXT)                                   \
    if(sqlite3_bind_int(                                                \
        STMT,                                                           \
        sqlite3_bind_parameter_index(STMT, ":"NAME),                    \
        TEXT                                                            \
    ) != SQLITE_OK) {osux_error("error binding int\n");exit(1);}

#define DB_BIND_DOUBLE(STMT, NAME, TEXT)                                \
    if (sqlite3_bind_double(                                            \
        STMT,                                                           \
        sqlite3_bind_parameter_index(STMT, ":"NAME),                    \
        TEXT                                                            \
    ) != SQLITE_OK) {osux_error("error binding double\n");exit(1);}

static int beatmap_db_insert(osux_beatmap *bm, osux_db *db)
{
    int ret;
    struct sqlite3_stmt *stmt;
    ret = sqlite3_prepare_v2(
        db->sqlite_db,
        "INSERT INTO beatmap "
        "(osu_beatmap_id, game_mode, audio_filename, diff_name,"
        "md5_hash, osu_filename,"
        "circles, sliders, spinners, last_modification, last_checked,"
        "approach_rate, circle_size, hp_drain, overall_diff,"
        "slider_velocity, stack_leniency, drain_time, total_time,"
        "preview_time, bpm_avg, bpm_max, bpm_min, local_offset,"
        "online_offset, already_played, last_played, ignore_hitsound,"
        "ignore_skin, disable_sb, disable_video, visual_override,"
        "mania_scroll_speed)"
        " VALUES"
        "(:osu_beatmap_id, :game_mode, :audio_filename, :diff_name,"
        ":md5_hash, :osu_filename,"
        ":circles, :sliders, :spinners, :last_modification, :last_checked,"
        ":approach_rate, :circle_size, :hp_drain, :overall_diff,"
        ":slider_velocity, :stack_leniency, :drain_time, :total_time,"
        ":preview_time, :bpm_avg, :bpm_max, :bpm_min, :local_offset,"
        ":online_offset, :already_played, :last_played, :ignore_hitsound,"
        ":ignore_skin, :disable_sb, :disable_video, :visual_override,"
        ":mania_scroll_speed)", -1, &stmt, NULL);

    if (ret != SQLITE_OK) {
        osux_error("%s\n", sqlite3_errmsg(db->sqlite_db));
        return -1;
    }

    DB_BIND_INT(stmt, "osu_beatmap_id", bm->BeatmapID);
    DB_BIND_INT(stmt, "game_mode", bm->Mode);
    DB_BIND_TEXT(stmt, "audio_filename", bm->AudioFilename);
    DB_BIND_TEXT(stmt, "md5_hash", bm->md5_hash);
    DB_BIND_TEXT(stmt, "osu_filename", bm->osu_filename);
    DB_BIND_TEXT(stmt, "diff_name", bm->Version);
    DB_BIND_INT(stmt, "circles", bm->circles);
    DB_BIND_INT(stmt, "sliders", bm->sliders);
    DB_BIND_INT(stmt, "spinners", bm->spinners);
    DB_BIND_INT(stmt, "last_modification", bm->last_modification);
    DB_BIND_INT(stmt, "last_checked", bm->last_checked);
    DB_BIND_DOUBLE(stmt, "approach_rate", bm->ApproachRate);
    DB_BIND_DOUBLE(stmt, "circle_size", bm->CircleSize);
    DB_BIND_DOUBLE(stmt, "hp_drain", bm->HPDrainRate);
    DB_BIND_DOUBLE(stmt, "overall_diff", bm->OverallDifficulty);
    DB_BIND_DOUBLE(stmt, "slider_velocity", bm->SliderMultiplier);
    DB_BIND_DOUBLE(stmt, "stack_leniency", bm->StackLeniency);
    DB_BIND_INT(stmt, "drain_time", bm->drain_time);
    DB_BIND_INT(stmt, "total_time", bm->total_time);
    DB_BIND_INT(stmt, "preview_time", bm->PreviewTime);
    DB_BIND_INT(stmt, "bpm_avg", bm->bpm_avg);
    DB_BIND_INT(stmt, "bpm_min", bm->bpm_min);
    DB_BIND_INT(stmt, "bpm_max", bm->bpm_max);
    DB_BIND_INT(stmt, "local_offset", bm->local_offset);
    DB_BIND_INT(stmt, "online_offset", bm->online_offset);
    DB_BIND_INT(stmt, "already_played", bm->already_played);
    DB_BIND_INT(stmt, "last_played", bm->last_played);
    DB_BIND_INT(stmt, "ignore_hitsound", bm->ignore_hitsound);
    DB_BIND_INT(stmt, "ignore_skin", bm->ignore_skin);
    DB_BIND_INT(stmt, "disable_sb", bm->disable_sb);
    DB_BIND_INT(stmt, "disable_video", bm->disable_video);
    DB_BIND_INT(stmt, "visual_override", bm->visual_override);
    DB_BIND_INT(stmt, "mania_scroll_speed", bm->mania_scroll_speed);

    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        osux_error("%s\n", sqlite3_errmsg(db->sqlite_db));
    }

    sqlite3_finalize(stmt);
    return 0;
}

char *osux_db_get_beatmap_path_by_hash(osux_db *db, const char *md5_hash)
{
    int err;
    osux_list *query_result = osux_list_new(0);
    err = db_query(db->sqlite_db, &fill_dict_list, query_result,
                   "select path from beatmap where md5_hash = '%s'", md5_hash);
    if (err < 0 || osux_list_size(query_result) == 0) {
        osux_list_free(query_result);
        return NULL;
    }

    if (osux_list_size(query_result) != 1) {
        g_warning("multiple result for beatmap query on field "
                  "'hash=%s';(hash collision?)", md5_hash);
    }

    osux_hashtable *dict = osux_list_get(query_result, 1);

    char *path;
    if (osux_hashtable_lookup(dict, "path", &path) < 0)
        path = NULL;
    osux_list_free(query_result);
    osux_hashtable_delete(dict);
    return path;
}

static int load_beatmap_from_disk(
    struct osux_db *db, const char *filepath, int base_path_length)
{
    osux_beatmap beatmap;

    if (osux_beatmap_init(&beatmap, filepath) < 0) {
        osux_error("Cannot load beatmap %s\n", filepath);
        return -1;
    }
    beatmap_db_insert(&beatmap, db);
    osux_beatmap_free(&beatmap);
    return 0;
}

static gboolean parse_beatmap_directory_recursive(
    const char *path, struct osux_db *db, int base_path_length, int level)
{
    GDir *dir = NULL;
    const gchar *entry_name;

    assert( db != NULL );

    if ((dir = g_dir_open(path, 0, NULL)) == NULL)
        return FALSE;
    if ((entry_name = g_dir_read_name(dir)) == NULL)
        return FALSE;

    do {
        char *new_path = g_strdup_printf("%s/%s", path, entry_name);
        if (g_file_test(new_path, G_FILE_TEST_IS_DIR)) {
            // with GLib "." and ".." directories are ommited on Unix platforms
            parse_beatmap_directory_recursive(new_path, db, base_path_length, level+1);
            g_free(new_path);
        } else {
            if (!string_have_extension(new_path, ".osu")) {
                g_free(new_path); // ignore non-osu file
                continue;
            }
            load_beatmap_from_disk(db, new_path, base_path_length);
            g_free(new_path);
        }
    } while ((entry_name = g_dir_read_name(dir)) != NULL);
    g_dir_close(dir);
    return TRUE;
}

int osux_db_create(struct osux_db **db_)
{
    int ret; struct osux_db *db;
    db = osux_malloc(sizeof*db);
    memset(db, 0, sizeof*db);
    ret = sqlite3_open(":memory:", &db->sqlite_db);
    if (ret) {
        osux_error("Can't open database: %s\n", sqlite3_errmsg(db->sqlite_db));
        sqlite3_close(db->sqlite_db);
        *db_ = NULL;
        return -1;
    }
    *db_ = db;
    return 0;
}

int osux_db_init(struct osux_db *db)
{
    char *errmsg;
    extern char _db_sql_data[];
    sqlite3_exec(db->sqlite_db, _db_sql_data, NULL, NULL, &errmsg);
    if (errmsg != NULL) {
        osux_error("%s\n", errmsg);
        sqlite3_free(errmsg);
        return -1;
    }
    return 0;
}

static int db_print_row(
    void *context, int column_count, char **column_text, char **column_name)
{
    FILE *output = context;
    for (int i = 0; i < column_count; ++i)
        fprintf(output, "%s: %s\n", column_name[i], column_text[i]);
    fprintf(output, "----\n");
    return 0;
}

int osux_db_query_print(FILE *output, const char *query, const osux_db *db)
{
    char *errmsg;
    sqlite3_exec(db->sqlite_db, query, db_print_row, output, &errmsg);
    if (NULL != errmsg) {
        osux_error("sqlite3_exec: %s\n", errmsg);
        sqlite3_free(errmsg);
        return -1;
    }
    return 0;
}

int osux_db_build(const char *directory_name, osux_db **db)
{
    if (NULL == db || NULL == directory_name) {
        osux_error("invalid argument");
        return -1;
    }

    osux_db_create(db);
    osux_db_init(*db);

    return parse_beatmap_directory_recursive(
        directory_name, *db, strlen(directory_name), 0) == TRUE ? 0 : -1;
}


/* NICE FUNCTION HAPPILY TAKEN FROM  https://www.sqlite.org/backup.html:
** This function is used to load the contents of a database file on disk
** into the "main" database of open database connection pInMemory, or
** to save the current contents of the database opened by pInMemory into
** a database file on disk. pInMemory is probably an in-memory database,
** but this function will also work fine if it is not.
**
** Parameter zFilename points to a nul-terminated string containing the
** name of the database file on disk to load from or save to. If parameter
** isSave is non-zero, then the contents of the file zFilename are
** overwritten with the contents of the database opened by pInMemory. If
** parameter isSave is zero, then the contents of the database opened by
** pInMemory are replaced by data loaded from the file zFilename.
**
** If the operation is successful, SQLITE_OK is returned. Otherwise, if
** an error occurs, an SQLite error code is returned.
*/
static int db_load_or_save(
    sqlite3 *pInMemory, const char *zFilename, bool isSave)
{
    int rc;                   /* Function return code */
    sqlite3 *pFile;           /* Database connection opened on zFilename */
    sqlite3_backup *pBackup;  /* Backup object used to copy data */
    sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
    sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

    /* Open the database file identified by zFilename. Exit early if this fails
    ** for any reason. */
    rc = sqlite3_open(zFilename, &pFile);
    if (SQLITE_OK == rc) {

        /* If this is a 'load' operation (isSave==0), then data is copied
        ** from the database file just opened to database pInMemory.
        ** Otherwise, if this is a 'save' operation (isSave==1), then data
        ** is copied from pInMemory to pFile.  Set the variables pFrom and
        ** pTo accordingly. */
        pFrom = (isSave ? pInMemory : pFile);
        pTo   = (isSave ? pFile     : pInMemory);

        /* Set up the backup procedure to copy from the "main" database of
        ** connection pFile to the main database of connection pInMemory.
        ** If something goes wrong, pBackup will be set to NULL and an error
        ** code and  message left in connection pTo.
        **
        ** If the backup object is successfully created, call backup_step()
        ** to copy data from pFile to pInMemory. Then call backup_finish()
        ** to release resources associated with the pBackup object.  If an
        ** error occurred, then  an error code and message will be left in
        ** connection pTo. If no error occurred, then the error code belonging
        ** to pTo is set to SQLITE_OK.
        */
        pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
        if (pBackup) {
            (void)sqlite3_backup_step(pBackup, -1);
            (void)sqlite3_backup_finish(pBackup);
        }
        rc = sqlite3_errcode(pTo);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
    (void) sqlite3_close(pFile);
    return rc;
}

int osux_db_save(const char *filename, const osux_db *db)
{
    assert( NULL != db );
    return db_load_or_save(db->sqlite_db, filename, true) != SQLITE_OK;
}

int osux_db_load(const char *filename, struct osux_db **db)
{
    if (g_access(filename, R_OK | W_OK) < 0) {
        osux_error("%s: %s\n", filename, strerror(errno));
        *db = NULL;
        return -1;
    }
    osux_db_create(db);
    return db_load_or_save(
        (*db)->sqlite_db, filename, false) != SQLITE_OK ? -1 : 0;
}

int osux_db_free(osux_db *db)
{
    if (NULL != db) {
        sqlite3_close(db->sqlite_db);
        osux_free(db);
        return 0;
    }
    return -1;
}

int osux_db_print_stat(FILE *outfile, const struct osux_db *db)
{
    fprintf(outfile, "Number of beatmaps: %d\n", db->db_stat.beatmap_count);
    fprintf(outfile, "Number of beatmap sets: %d\n",
            db->db_stat.beatmap_set_count);
    return 0;
}

int osux_db_dump(FILE *outfile, const osux_db *db)
{
    (void) db_query(db->sqlite_db, db_print_row, outfile, "select * from beatmap");
    return 0;
}

