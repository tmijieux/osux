#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

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
    struct osux_beatmap *bm;
    
    f = fopen(path, "r");
    if (NULL != f) {
        printf("db_parse: osu file: %s\n", path);
    } else {
        osux_error("osu file BUG: %s\n", path);
        return -1;
    }

    md5 = osux_md5_hash_file(f);
    md5_hash = osux_md5_string(md5);
    free(md5);
    bm = osux_beatmap_open(filename);

    errmsg = NULL;
    sqlite3_exec(odb->db,
                 "INSERT INTO ",

                 NULL, NULL, &errmsg);
    osux_beatmap_close(bm);
    if (NULL != errmsg) {
        osux_error("%s\n", errmsg);
        exit(EXIT_FAILURE);
    }
        
    sqlite3_free(errmsg);
 
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

extern char _db_sql_data[];

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
    sqlite3_exec(odb->db, _db_sql_data,
                 NULL, NULL, &errmsg);
    if (errmsg != NULL) {
        osux_error("%s", errmsg);
        exit(EXIT_FAILURE);
    }

}

int osux_db_build(const char *directory_name, struct osudb *odb)
{
    osux_db_init(odb);
    return parse_beatmap_directory_rec(directory_name, odb,
                                       strlen(directory_name), 0);
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
int osux_db_load_or_save(sqlite3 *pInMemory, const char *zFilename, bool isSave){
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

int osux_db_write(const char *filename, const struct osudb *odb)
{
    FILE *f;
    assert( NULL != odb );
    return osux_load_or_save(odb->db, filename, true) != SQLITE_OK;
}


int osux_db_read(const char *filename, struct osudb *odb)
{
    FILE *f;
    assert( NULL != odb );
    memset(odb, 0, sizeof*odb);
    osux_db_init(odb);
    return osux_load_or_save(odb->db, filename, false) != SQLITE_OK;
}

void osux_db_free(struct osudb *odb)
{
    if (NULL != odb) {
        sqlite3_close(odb->db);
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
