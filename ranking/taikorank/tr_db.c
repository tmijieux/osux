/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <mysql/mysql.h>

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"

#include "mods.h"
#include "print.h"

#define TR_DB_IP     "localhost"
#define TR_DB_LOGIN  "root"
#define TR_DB_PASSWD "NOPE"

#define TR_DB_NAME "taiko_rank"
#define TR_DB_USER "tr_user"
#define TR_DB_BMS  "tr_beatmap_set"
#define TR_DB_DIFF "tr_diff"

static MYSQL * sql;

static int new_rq(MYSQL * sql, const char * rq, ...);
static int tr_db_get_id(MYSQL * sql, char * table, char * cond);
static char * tr_db_escape_str(MYSQL * sql, const char * src);

//-------------------------------------------------

__attribute__((constructor))
static void tr_db_init(void)
{
  sql = mysql_init(NULL);

  if (sql == NULL)
    {
      fprintf(OUTPUT_ERR, "Error: mysql init\n");
      return;
    }

  if (NULL == mysql_real_connect(sql,
				 TR_DB_IP, TR_DB_LOGIN, TR_DB_PASSWD,
				 NULL, 0, NULL, 0)) 
    {
      fprintf(OUTPUT_ERR, "%s\n", mysql_error(sql));
      mysql_close(sql);
      sql = NULL;
      return;
    }  
}

//-------------------------------------------------

__attribute__((destructor))
static void tr_db_exit(void)
{
  if (sql != NULL)
    mysql_close(sql);
}

//-------------------------------------------------

static int new_rq(MYSQL * sql, const char * rq, ...)
{
  va_list va;
  va_start(va, rq);
  char * buf = NULL;
  vasprintf(&buf, rq, va);
  va_end(va);
  //fprintf(OUTPUT_ERR, "%s\n", buf);

  if (mysql_query(sql, buf)) 
    {
      fprintf(stderr, "%s\n", mysql_error(sql));
      free(buf);
      return -1;
    }

  free(buf);
  return 0;
}

//-------------------------------------------------

static int tr_db_get_id(MYSQL * sql, char * table, char * cond)
{
  new_rq(sql, "SELECT * FROM %s WHERE %s;", table, cond);
  
  MYSQL_RES * result = mysql_store_result(sql);
  MYSQL_ROW row = mysql_fetch_row(result);
  
  int id = -1;
  if (row != NULL)
    id = atoi(row[0]);
  return id;
}

//-------------------------------------------------

static char * tr_db_escape_str(MYSQL * sql, const char * src)
{
  unsigned int l = strlen(src);
  char * dst = malloc(sizeof(char) * (2 * l + 1));
  mysql_real_escape_string(sql, dst, src, l);
  return dst;
}


//-------------------------------------------------

void tr_db_add(struct tr_map * map)
{
  if (sql == NULL)
    {
      fprintf(OUTPUT_ERR, "Couldn't connect to DB. Data won't be stored.");
      return;
    }

  // escape char
  char * map_title   = tr_db_escape_str(sql, map->title);
  char * map_artist  = tr_db_escape_str(sql, map->artist);
  char * map_source  = tr_db_escape_str(sql, map->source);
  char * map_creator = tr_db_escape_str(sql, map->creator);
  char * map_diff    = tr_db_escape_str(sql, map->diff);

  // use db
  new_rq(sql, "USE %s;", TR_DB_NAME);
  
  // creator
  char * cond = NULL;
  asprintf(&cond, "name = '%s'", map_creator);

  int user_id = tr_db_get_id(sql, TR_DB_USER, cond);
  if (user_id < 0)
    {
      new_rq(sql, "INSERT INTO %s(name) VALUES('%s');",
	     TR_DB_USER, map_creator);
      user_id = tr_db_get_id(sql, TR_DB_USER, cond);
      fprintf(OUTPUT_INFO, "New user: %s, ID: %d\n", map_creator, user_id);
    }
  free(cond);
  cond = NULL;

  // beatmap_set
  asprintf(&cond, "creator_ID = %d and artist = '%s' and title = '%s'",
	   user_id, map_artist, map_title);

  int bms_id = tr_db_get_id(sql, TR_DB_BMS, cond);
  if (bms_id < 0)
    {
      new_rq(sql, "INSERT INTO %s(artist, title, source, creator_ID)"
	     "VALUES('%s', '%s', '%s', %d);",
	     TR_DB_BMS, map_artist, map_title, map_source, user_id);
      bms_id = tr_db_get_id(sql, TR_DB_BMS, cond);
      fprintf(OUTPUT_INFO, "New beatmap: %s - %s ID: %d\n",
	      map_artist, map_title, bms_id);
    }
  free(cond);
  cond = NULL;

  // diff
  asprintf(&cond, "bms_ID = %d and diff_name = '%s'",
	   bms_id, map_diff);

  int diff_id = tr_db_get_id(sql, TR_DB_DIFF, cond);
  if (diff_id < 0)
    {
      new_rq(sql, "INSERT INTO %s(diff_name, bms_ID, "
	     "density_star, reading_star, pattern_star, "
	     "accuracy_star, final_star) "
	     "VALUES('%s', %d, %g, %g, %g, %g, %g);",
	     TR_DB_DIFF, map_diff, bms_id, map->density_star,
	     map->reading_star, map->pattern_star,
	     map->accuracy_star, map->final_star);
      diff_id = tr_db_get_id(sql, TR_DB_DIFF, cond);
      fprintf(OUTPUT_INFO, "New diff: %s ID: %d\n",
	      map_diff, diff_id);
    }
  else
    {
      new_rq(sql, "UPDATE %s "
	     "SET density_star = %g, reading_star = %g, pattern_star = %g,"
	     "accuracy_star = %g, final_star = %g"
	     "WHERE ID = %d;",
	     TR_DB_DIFF, map->density_star, map->reading_star,
	     map->pattern_star, map->accuracy_star,
	     map->final_star, diff_id);
      fprintf(OUTPUT_INFO, "Updated diff: %s ID: %d\n",
	      map_diff, diff_id);
    }
  free(cond);
  
  free(map_title);
  free(map_artist);
  free(map_source);
  free(map_creator);
  free(map_diff);
}

