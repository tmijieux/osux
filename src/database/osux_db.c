#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

#include <sqlite3.h>

#include "beatmap/parser/parser.h"
#include "util/list.h"
#include "util/hash_table.h"
#include "util/split.h"
#include "util/md5.h"
#include "util/data.h"
#include "util/error.h"
#include "osux_db.h"

static int osux_db_load_or_save(
    sqlite3 *pInMemory, const char *zFilename, bool isSave);

struct osux_db {
    uint32_t beatmaps_number;
    sqlite3 *sqlite_db;
    bool db_hashed;
    struct hash_table *hashmap;

};

static int
load_beatmap(struct osux_db *db, const char *filename, int base_path_length)
{
    FILE *f;
    char *errmsg;
    osux_beatmap *bm;

    f = fopen(filename, "r");
    if (NULL != f) {
        printf("db_parse: osu file: %s\n", filename);
    } else {
        osux_error("osu file BUG: %s\n", filename);
        return -1;
    }

    (void) osux_beatmap_open(filename, &bm);
    (void) osux_md5_hash_file(f, bm->md5_hash);

    errmsg = NULL;
    sqlite3_exec(db->sqlite_db,
                 "INSERT INTO ",
                 NULL, NULL, &errmsg);
    osux_beatmap_close(bm);
    if (NULL != errmsg) {
        osux_error("%s\n", errmsg);
        sqlite3_free(errmsg);
        exit(EXIT_FAILURE);
    }

    sqlite3_free(errmsg);

    fclose(f);
    return 0;
}

static int
parse_beatmap_directory_rec(const char *name, struct osux_db *db,
                            int base_path_length, int level)
{
    DIR *dir;
    struct dirent *entry;

    assert( db != NULL );

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
            parse_beatmap_directory_rec(path, db, base_path_length, level+1);
            free(path);
        } else {
            if (!string_have_extension(path, ".osu")) {
                free(path);
                continue;
            }
            load_beatmap(db, path, base_path_length);
            free(path);
        }
    } while ((entry = readdir(dir)) != NULL);
    closedir(dir);

    return 0;
}

int osux_db_create(struct osux_db **db)
{
    int ret;
    *db = osux_malloc(sizeof**db);
    memset(*db, 0, sizeof**db);

    ret = sqlite3_open(":memory:", &(*db)->sqlite_db);
    if (ret) {
        osux_error("Can't open database: %s\n", sqlite3_errmsg((*db)->sqlite_db));
        sqlite3_close((*db)->sqlite_db);
        return -1;
    }
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

static int db_print_row(void *context, int column_count,
                        char **column_text, char **column_name)
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

    return 0;
    return parse_beatmap_directory_rec(
        directory_name, *db, strlen(directory_name), 0);
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
static int osux_db_load_or_save(
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
    return osux_db_load_or_save(db->sqlite_db, filename, true) != SQLITE_OK;
}

int osux_db_load(const char *filename, struct osux_db **db)
{
    osux_db_create(db);
    return osux_db_load_or_save((*db)->sqlite_db, filename, false) != SQLITE_OK;
}

int osux_db_free(osux_db *db)
{
    if (NULL != db) {
        sqlite3_close(db->sqlite_db);
        if (db->db_hashed) {
            ht_free(db->hashmap);
        }
        osux_free(db);
        return 0;
    }
    return -1;
}

int osux_db_dump(FILE *outfile, const osux_db *db)
{
    fprintf(outfile, "Number of maps: %d\n", db->beatmaps_number);
    fprintf(outfile, "%s:%s NOT IMPLEMENTED\n",
            __FILE__, __PRETTY_FUNCTION__);
    return 0;
}

int osux_db_hash(osux_db *db)
{
    if (NULL == db)
        return -1;

    db->hashmap = ht_create(db->beatmaps_number, NULL);
    db->db_hashed = true;

    return 0;
}

const char*
osux_db_relpath_by_hash(osux_db *db, const char *hash)
{
    char *ret = NULL;
    ht_get_entry(db->hashmap, hash, &ret);
    return ret;
}

osux_beatmap *osux_db_get_beatmap_by_hash(osux_db *db, const char *hash)
{
    osux_beatmap *bm;
    char *path = osux_prefix_path(osux_get_song_path(),
                                  osux_db_relpath_by_hash(db, hash));

    (void) osux_beatmap_open(path, &bm);
    free(path);

    return bm;
}
