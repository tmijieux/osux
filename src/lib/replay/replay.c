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

#include "osux/game_mode.h"
#include "osux/util.h"
#include "osux/list.h"
#include "osux/read.h"
#include "osux/mods.h"
#include "osux/replay.h"
#include "osux/buffer_reader.h"
#include "osux/keys.h"

#define read_string(buf_ptr_, file_) read_string_ULEB128(buf_ptr_, file_)

static int replay_data_init(osux_replay_data *d, char *datastr)
{
    gchar **split = g_strsplit(datastr, "|", 0);
    unsigned size = strsplit_size(split);

    if (size != 4) {
        g_strfreev(split);
        return -OSUX_ERR_REPLAY_DATA;
    }

    d->previous_time = g_ascii_strtoull(split[0], NULL, 10);
    d->x = g_ascii_strtod(split[1], NULL);
    d->y = g_ascii_strtod(split[2], NULL);
    d->keys = atoi(split[3]);
    d->time_offset = 0;

    g_strfreev(split);
    return 0;
}

static int parse_replay_data(osux_replay *r, char const *data)
{
    int err = 0;
    if (data == NULL)
        return 0; // not an error: replay with no data can exist

    char **data_split = g_strsplit(data, ",", 0);
    unsigned size = strsplit_size(data_split);

    ALLOC_ARRAY(r->data, r->data_count, size);
    for (unsigned i = 0; i < size; ++i) {
        if (i == size-1 && !strcmp(data_split[i], "")) {
            // see parse_life_graph()
            -- r->data_count;
            break;
        }
        if ((err = replay_data_init(&r->data[i], data_split[i])) < 0)
            break;
        osux_replay_data *d = &r->data[i];
        if (i > 0)
            d->time_offset = (d-1)->time_offset + d->previous_time;
    }
    g_strfreev(data_split);
    return err;
}

#define TICKS_PER_SECONDS 10000000L
#define TICKS_AT_EPOCH  621355968000000000L
// enum wont work here because C enum cannot be anything else than 'int' type
// and TICKS_AT_EPOCH may be truncated by the compiler :D

static time_t from_win_timestamp(uint64_t ticks)
{
    return (ticks - TICKS_AT_EPOCH) / TICKS_PER_SECONDS;
}

static void print_date(FILE *f, time_t t)
{
    static gboolean locale_is_set = FALSE;

    if (!locale_is_set) {
        setlocale(LC_ALL, ""); // TODO: move this
        locale_is_set = TRUE;
    }

    GDateTime *dateTime = g_date_time_new_from_unix_local(t);
    g_assert(dateTime != NULL);

    gchar *dateStr = g_date_time_format(dateTime, "%c");
    fprintf(f, "date: %s\n", dateStr);
    g_free(dateStr);
    g_date_time_unref(dateTime);
}

static int replay_life_init(osux_replay_life *life, char const *lifestr)
{
    gchar **split = g_strsplit(lifestr, "|", 0);
    unsigned size = strsplit_size(split);
    if (size != 2) {
        g_strfreev(split);
        return -OSUX_ERR_REPLAY_LIFE_BAR;
    }

    life->time_offset = g_ascii_strtoull(split[0], NULL, 10);
    life->life_amount = g_ascii_strtod(split[1], NULL);

    g_strfreev(split);
    return 0;
}

static int parse_life_graph(osux_replay *r, char const *life_graph)
{
    int err = 0;

    if (life_graph == NULL) {
        // not an error:
        // 'pure score' .osr file have no data and no life graph
        return 0;
    }

    char **life_split = g_strsplit(life_graph, ",", 0);
    unsigned size = strsplit_size(life_split);

    ALLOC_ARRAY(r->life, r->life_count, size);
    for (unsigned i = 0; i < size; ++i) {
        if (i == size-1 && !strcmp(life_split[i], "")) {
            // this often happens: a comma get appended in the end
            // but glib will still report a empty token
            -- r->life_count;
            break;
        }
        if ((err = replay_life_init(&r->life[i], life_split[i])) < 0)
            break;
    }
    g_strfreev(life_split);
    return err;
}

#define READ_S(handle, var)  obr_read_string(&(handle), &(var))
#define READ_V(handle, var)  obr_read(&(handle), &(var), sizeof (var))

int osux_replay_init(osux_replay *r, char const *filepath)
{
    int err = 0;
    gsize length;  gchar *contents;
    if (!g_file_get_contents(filepath, &contents, &length, NULL))
        return -OSUX_ERR_FILE_ERROR;

    osux_buffer_reader br;
    osux_buffer_reader_init(&br, contents, length);

    memset(r, 0, sizeof *r);

    READ_V(br, r->game_mode);
    READ_V(br, r->game_version);

    READ_S(br, r->beatmap_hash);
    READ_S(br, r->player_name);
    READ_S(br, r->replay_hash);

    READ_V(br, r->_300);
    READ_V(br, r->_100);
    READ_V(br, r->_50);
    READ_V(br, r->_geki);
    READ_V(br, r->_katu);
    READ_V(br, r->_miss);

    READ_V(br, r->score);
    READ_V(br, r->max_combo);
    READ_V(br, r->fc);
    READ_V(br, r->mods);

    char *life_graph = NULL;
    READ_S(br, life_graph);
    if ((err = parse_life_graph(r, life_graph)) < 0) {
        osux_replay_free(r);
        osux_buffer_reader_free(&br);
        return err;
    }
    g_free(life_graph);

    uint64_t ticks;
    READ_V(br, ticks);
    r->timestamp = from_win_timestamp(ticks);

    READ_V(br, r->replay_length);
    char *data = NULL;
    obr_read_lzma(&br, &data, r->replay_length);
    if ((err = parse_replay_data(r, data)) < 0) {
        osux_replay_free(r);
        osux_buffer_reader_free(&br);
        return err;
    }
    g_free(data);
    osux_buffer_reader_free(&br);
    return 0;
}

void osux_replay_print(osux_replay const *r, FILE *f)
{
    fprintf(f, "GameMode: %u\n", r->game_mode);
    fprintf(f, "Game version: %u\n", r->game_version);
    fprintf(f, "Beatmap hash: %s\n", r->beatmap_hash);
    fprintf(f, "Player name: %s\n", r->player_name);
    fprintf(f, "Replay hash: %s\n", r->replay_hash);

    fprintf(f, "300: %u\n", r->_300);
    fprintf(f, "100: %u\n", r->_100);
    fprintf(f, "50: %u\n", r->_50);
    fprintf(f, "geki: %u\n", r->_geki);
    fprintf(f, "katu: %u\n", r->_katu);

    fprintf(f, "miss: %u\n", r->_miss);
    fprintf(f, "Score: %u\n", r->score);
    fprintf(f, "MaxCombo: %u\n", r->max_combo);
    fprintf(f, "FullCombo: %u\n", r->fc);
    fputs("Mods: ", f);  mod_print(f, r->mods); fputs("\n", f);
    fprintf(f, "Life Graph: "/*%s\n", r->lifebar_graph*/);

    for (unsigned i = 0; i < r->life_count; ++i) {
        osux_replay_life *l = &r->life[i];
        g_fprintf(f, "%"G_GUINT64_FORMAT"|%lg,",
                  l->time_offset, l->life_amount);
    }
    fprintf(f, "\n");

    print_date(f, r->timestamp);

    fprintf(f, "Replay size: %u bytes\n\n", r->replay_length);
    fprintf(f, "Data:\n");

    for (unsigned i = 0; i < r->data_count; ++i) {
        osux_replay_data *d = &r->data[i];
    	g_fprintf(f, "%"G_GINT64_FORMAT"|%g|%g|%u\n",
                  d->previous_time, d->x, d->y, d->keys);
    }
}

void osux_replay_free(osux_replay *r)
{
    g_free(r->beatmap_hash);
    g_free(r->player_name);
    g_free(r->replay_hash);
    g_free(r->life);
    g_free(r->data);
}

