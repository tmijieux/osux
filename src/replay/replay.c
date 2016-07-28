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

#include <glib.h>
#include <glib/gprintf.h>

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <locale.h>

#include <beatmap/beatmap.h> // game modes
#include <util/uleb128.h>
#include <util/list.h>
#include <util/split.h>
#include <util/read.h>
#include <mod/mods.h>

#include "replay.h"
#include "xz_decomp.h"

#define read_string(buf_ptr_, file_) read_string_ULEB128(buf_ptr_, file_)

#define DATE_MAXSIZE 200

static unsigned int parse_replay_data(FILE *f, struct replay_data **repdata)
{
    gchar *uncomp, **tab;
    
    lzma_decompress(f, (uint8_t**) &uncomp);
    if (NULL == uncomp) {
        *repdata = NULL;
        return (unsigned) -1;
    }
    
    tab = g_strsplit(uncomp, ",", 0);
    free(uncomp);
	gsize tabSize = 0;
	while (tab[tabSize] != NULL)
		tabSize++;
    
    *repdata = calloc(sizeof(**repdata), tabSize);
    for (unsigned int i = 0; i < tabSize; ++i) {
		gchar **split = g_strsplit(tab[i], "|", 0);
		(*repdata)[i].previous_time = g_ascii_strtoull(split[0], NULL, 10);
		(*repdata)[i].x = g_ascii_strtod(split[1], NULL);
		(*repdata)[i].y = g_ascii_strtod(split[2], NULL);
		(*repdata)[i].keys = atoi(split[3]);
		g_strfreev(split);
    }
	g_strfreev(tab);
	
    return tabSize;
}

#define TICKS_PER_SECONDS 10000000L
#define TICKS_AT_EPOCH  621355968000000000L

static time_t from_win_timestamp(uint64_t ticks)
{
    return (ticks - TICKS_AT_EPOCH) / TICKS_PER_SECONDS;
}

static void print_date(FILE *f, time_t t)
{
    static gboolean locale_is_set = FALSE;
    
    if (!locale_is_set) {
        setlocale(LC_TIME, ""); // TODO: move this
        locale_is_set = TRUE;
    }

	GDateTime *dateTime = g_date_time_new_from_unix_local(t);
	g_assert(dateTime != NULL);
	gchar *dateStr = g_date_time_format(dateTime, "%c");
    fprintf(f, "date: %s\n", dateStr);
	g_free(dateStr);
	g_date_time_unref(dateTime);
}

struct replay *replay_parse(FILE *f)
{
    char **life;
    uint64_t ticks;
    struct replay *r = calloc(sizeof(*r), 1);
    r->invalid = false;
    rewind(f);

    xfread(&r->game_mode, 1, 1, f);
    xfread(&r->game_version, 4, 1, f);

    read_string(&r->bm_md5_hash, f);
    read_string(&r->player_name, f);
    read_string(&r->replay_md5_hash, f);
    
    xfread(&r->_300,  2, 1, f);
    xfread(&r->_100,  2, 1, f);
    xfread(&r->_50,   2, 1, f);
    xfread(&r->_geki, 2, 1, f);
    xfread(&r->_katu, 2, 1, f);
    xfread(&r->_miss, 2, 1, f);

    xfread(&r->score, 4, 1, f);
    xfread(&r->max_combo, 2, 1, f);
    xfread(&r->fc, 1, 1, f);
    xfread(&r->mods, 4, 1, f);

    read_string(&r->lifebar_graph, f);
	
    r->replife_size = 0;
	life = g_strsplit(r->lifebar_graph, ",", 0);
	while (life[r->replife_size] != NULL)
		++ r->replife_size;
	
    for (unsigned int i = 0; i < r->replife_size; ++i) {
		gchar **split = g_strsplit(life[i], "|", 0);
		r->replife[i].time_offset = g_ascii_strtoull(split[0], NULL, 10);
		r->replife[i].life_amount = g_ascii_strtod(split[1], NULL);
		g_strfreev(split);
    }
	g_strfreev(life);

    xfread(&ticks, 8, 1, f);
    r->timestamp = from_win_timestamp(ticks);

    xfread(&r->replay_length, 4, 1, f);
    if ((r->repdata_count =
        parse_replay_data(f, &r->repdata)) == (unsigned)-1) {
        r->invalid = 1;
    }
    
    return r;
}

void replay_print(FILE *f, const struct replay *r)
{
    fprintf(f, "game mode: %u\n", r->game_mode);
    fprintf(f, "game version: %u\n", r->game_version);
    fprintf(f, "beatmap md5 hash: %s\n", r->bm_md5_hash);
    fprintf(f, "player name: %s\n", r->player_name);
    fprintf(f, "replay md5 hash: %s\n", r->replay_md5_hash);

    fprintf(f, "300: %u\n", r->_300);
    fprintf(f, "100: %u\n", r->_100);
    fprintf(f, "50: %u\n", r->_50);
    fprintf(f, "geki: %u\n", r->_geki);
    fprintf(f, "katu: %u\n", r->_katu);

    fprintf(f, "miss: %u\n", r->_miss);
    fprintf(f, "score: %u\n", r->score);
    fprintf(f, "max combo: %u\n", r->max_combo);
    fprintf(f, "Full combo: %u\n", r->fc);
    fputs("mods: ", f);  mod_print(f, r->mods); fputs("\n", f);  
    fprintf(f, "lifebar_graph: "/*%s\n", r->lifebar_graph*/);

    for (unsigned int i = 0; i < r->replife_size; ++i) {
        struct replay_life *rl = &r->replife[i];
        g_fprintf(f, "%"G_GUINT64_FORMAT"|%lg%s", rl->time_offset, rl->life_amount,
                i == r->replife_size-1 ? "" : ",");
    }
    fputs("\n", f);

    print_date(f, r->timestamp);

    fprintf(f, "replay length: %u bytes\n", r->replay_length);
    fprintf(f, "\n");
    fprintf(f, "__ DATA __ :\n");

    for (unsigned int i = 0; i < r->repdata_count; ++i) {
    	struct replay_data *rd = &r->repdata[i];
    	g_fprintf(f, "%"G_GINT64_FORMAT"|%g|%g|%u\n", rd->previous_time, rd->x, rd->y, rd->keys);
    }
}

void replay_free(struct replay *r)
{
    free(r->bm_md5_hash);
    free(r->player_name);
    free(r->replay_md5_hash);
    free(r->lifebar_graph);
    free(r->replife);
    free(r->repdata);
    free(r);
}


