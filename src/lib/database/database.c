#include <glib.h>
#include "osux/error.h"
#include "osux/database.h"

#define CALLBACK_SQLITE

static int CALLBACK_SQLITE print_row(
    void *user_data, int col_count, char **col_text, char **col_name)
{
    FILE *out = (FILE*) user_data;
    for (int i = 0; i < col_count; ++i)
        fprintf(out, "%s: %s\n", col_name[i], col_text[i]);
    fprintf(out, "---\n");
    return 0;
}

static int CALLBACK_SQLITE fill_result(
    void *user_data, int col_count, char **col_text, char **col_name)
{
    osux_hashtable *dict = osux_hashtable_new(0);
    osux_list *query_result = (osux_list*) user_data;

    for (int i = 0; i < col_count; ++i)
        if (col_text[i] != NULL)
            osux_hashtable_insert(dict, col_name[i], col_text[i]);
    osux_list_append(query_result, dict);
}

static sqlite3 *get_handle(osux_database *db)
{
    if (db->in_memory)
        return db->mem_handle;
    return db->file_handle;
}

int osux_database_exec_prepared_query(osux_database *db, osux_list *query_result)
{
    int ret = sqlite3_step(db->prepared_query);

    while (ret != SQLITE_DONE) {
        if (ret != SQLITE_ROW) {
            osux_debug("%s\n", sqlite3_errmsg(get_handle(db)));
            return OSUX_ERR_DATABASE;
        }
        osux_hashtable *dict = osux_hashtable_new(0);

        int col_count = sqlite3_data_count(db->prepared_query);
        for (int i = 0; i < col_count; ++i) {
            char const *col_text = sqlite3_column_text(db->prepared_query, i);
            if (col_text != NULL) {
                char const *col_name = sqlite3_column_name(db->prepared_query, i);
                osux_hashtable_insert(dict, col_name, g_strdup(col_text));
            }
        }
        osux_list_append(query_result, dict);
    }
    sqlite3_reset(db->prepared_query);
    sqlite3_clear_bindings(db->prepared_query);
}

int osux_database_exec_query(
    osux_database *db, char const *query, osux_list *query_result)
{
    char *errmsg = NULL;
    int ret = sqlite3_exec(get_handle(db), query, fill_result,
                           (void*) query_result, &errmsg);
    if (ret != SQLITE_OK && errmsg != NULL) {
        osux_debug("%s\n", errmsg);
        sqlite3_free(errmsg);
        return OSUX_ERR_DATABASE;
    }
    return 0;
}

int osux_database_print_query(osux_database *db, char const *query, FILE *out)
{
    char *errmsg = NULL;
    int ret = sqlite3_exec(get_handle(db), query, print_row,
                           (void*) out, &errmsg);
    if (ret != SQLITE_OK && errmsg != NULL) {
        osux_debug("%s\n", errmsg);
        sqlite3_free(errmsg);
        return OSUX_ERR_DATABASE;
    }
    return 0;
}

int osux_database_bind_int(osux_database *db, char const *name, int i)
{
    int index = sqlite3_bind_parameter_index(db->prepared_query, name);
    if (sqlite3_bind_int(db->prepared_query, index, i) != SQLITE_OK) {
        osux_debug("%s\n", sqlite3_errmsg(get_handle(db)));
        return OSUX_ERR_DATABASE;
    }
    return 0;
}

int osux_database_bind_double(osux_database *db, char const *name, double d)
{
    int index = sqlite3_bind_parameter_index(db->prepared_query, name);
    if (sqlite3_bind_double(db->prepared_query, index, d) != SQLITE_OK) {
        osux_debug("%s\n", sqlite3_errmsg(get_handle(db)));
        return OSUX_ERR_DATABASE;
    }
    return 0;
}

int osux_database_bind_string(osux_database *db, char const *name, char const *str)
{
    int index = sqlite3_bind_parameter_index(db->prepared_query, name);
    if (sqlite3_bind_text(db->prepared_query, index, str, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        osux_debug("%s\n", sqlite3_errmsg(get_handle(db)));
        return OSUX_ERR_DATABASE;
    }
    return 0;
}

int osux_database_init(osux_database *db, char const *file_path)
{
    memset(db, 0, sizeof *db);
    db->in_memory = false;

    int ret = sqlite3_open(file_path, &db->file_handle);
    if (ret) {
        osux_debug("%s\n", sqlite3_errmsg(db->file_handle));
        sqlite3_close(db->file_handle);
        return OSUX_ERR_DATABASE;
    }
}

static int load_or_save_memory(osux_database *db, bool is_save)
{
    sqlite3_backup *pBackup;  /* Backup object used to copy data */
    sqlite3 *pTo;             /* Database to copy to (pFile or pInMemory) */
    sqlite3 *pFrom;           /* Database to copy from (pFile or pInMemory) */

    pFrom = (is_save ? db->mem_handle  : db->file_handle);
    pTo   = (is_save ? db->file_handle : db->mem_handle);

    pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
    if (pBackup) {
        (void) sqlite3_backup_step(pBackup, -1);
        (void) sqlite3_backup_finish(pBackup);
    }
    return sqlite3_errcode(pTo);
}

static void save_from_memory(osux_database *db)
{
    if (db->in_memory) {
        load_or_save_memory(db, true);
        sqlite3_close(db->mem_handle);
        db->mem_handle = NULL;
        db->in_memory = false;
    }
}

static int load_to_memory(osux_database *db)
{
    if (!db->in_memory) {
        int ret = sqlite3_open(":memory:", &db->mem_handle);
        if (ret) {
            osux_debug("%s\n", sqlite3_errmsg(db->mem_handle));
            sqlite3_close(db->mem_handle);
            return OSUX_ERR_DATABASE;
        }
        load_or_save_memory(db, false);
        db->in_memory = true;
    }
}

void osux_database_free(osux_database *db)
{
    if (db->in_memory)
        save_from_memory(db);
    sqlite3_close(db->file_handle);
}

int osux_database_prepare_query(osux_database *db, char const *query)
{
    int ret;
    if (&db->prepared_query != NULL)
        sqlite3_finalize(db->prepared_query);
    
    ret = sqlite3_prepare_v2(get_handle(db), query, -1,
                             &db->prepared_query, NULL);
    if (ret != SQLITE_OK) {
        osux_debug("%s\n", sqlite3_errmsg(get_handle(db)));
        return OSUX_ERR_DATABASE;
    }
    return 0;
}
