#ifndef OSUX_DATABASE_H
#define OSUX_DATABASE_H

#include <sqlite3.h>
#include <stdbool.h>
#include "osux/list.h"

typedef struct osux_database_ {
    
    sqlite3 *mem_handle;
    sqlite3 *file_handle;
    bool in_memory;
    char *file_path;

    sqlite3_stmt *prepared_query;
} osux_database;



int osux_database_init(osux_database *db, char const *file_path);
void osux_database_free(osux_database *db);

int osux_database_exec_query(
    osux_database *db, char const *query, osux_list *query_result);

int osux_database_prepare_query(osux_database *db, char const *query);
int osux_database_bind_int(osux_database *db, char const *name, int i);
int osux_database_bind_double(osux_database *db, char const *name, double d);
int osux_database_bind_string(osux_database *db, char const *name, char const *str);
int osux_database_exec_prepared_query(osux_database *db, osux_list *query_result);



#endif // OSUX_DATABASE_H
