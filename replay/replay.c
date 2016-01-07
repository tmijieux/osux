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

#include <stdlib.h>
#include <unistd.h>

#include "replay.h"

#include <beatmap/beatmap.h> // game modes
#include <util/uleb128/uleb128.h>
#include <util/list/list.h>
#include <util/string/split.h>
#include <general/mods.h>


#include "mono.h"
#include "xz_decomp.h"

static unsigned int parse_replay_data(FILE *f, struct replay_data **repdata)
{
    char *uncomp, **tab;
    unsigned int repdata_count;
    
    lzma_decompress(f, (uint8_t**) &uncomp);
    repdata_count = string_split(uncomp, ",", &tab);
    free(uncomp);
    
    *repdata = calloc(sizeof(**repdata), repdata_count);
    for (int i = 0; i < repdata_count; ++i) {
	sscanf(tab[i], "%ld|%lg|%lg|%d",
	       &(*repdata)[i].previous_time,
	       &(*repdata)[i].x,
	       &(*repdata)[i].y,
    	       &(*repdata)[i].keys);
	free(tab[i]);
    }
    free(tab);
    return repdata_count;
}

static void read_string(char **buf, FILE *f)
{
    uint8_t h;
    fread(&h, 1, 1, f);
    if (h == 0x0B) {
	uint64_t s = read_ULEB128(f);
	*buf = malloc(s+1);
	fread(*buf, s, 1, f);
	(*buf)[s] = 0;
    } else {
	*buf = NULL;
    }
}


struct replay *replay_parse(FILE *f)
{
    rewind(f);

    struct replay *r = calloc(sizeof(*r), 1);

    
    fread(&r->game_mode, 1, 1, f);
    fread(&r->game_version, 4, 1, f);

    read_string(&r->bm_md5_hash, f);
    read_string(&r->player_name, f);
    read_string(&r->replay_md5_hash, f);

    fread(&r->_300,  2, 1, f);
    fread(&r->_100,  2, 1, f);
    fread(&r->_50,   2, 1, f);
    fread(&r->_geki, 2, 1, f);
    fread(&r->_katu, 2, 1, f);
    fread(&r->_miss, 2, 1, f);

    fread(&r->score, 4, 1, f);
    fread(&r->max_combo, 2, 1, f);
    fread(&r->fc, 1, 1, f);
    fread(&r->mods, 4, 1, f);

    read_string(&r->lifebar_graph, f);
    
    fread(&r->timestamp, 8, 1, f);
    fread(&r->replay_length, 4, 1, f);

    r->repdata_count = parse_replay_data(f, &r->repdata);
    
    return r;
}

int cs_timestamp_string(uint64_t timestamp);

#define TICKS_PER_SECONDS 10000000L
#define LOL l

void replay_print(FILE *f, const struct replay *r)
{
    fprintf(f, "game mode: %hhu\n", r->game_mode);
    fprintf(f, "game version: %u\n", r->game_version);
    fprintf(f, "beatmap md5 hash: %s\n", r->bm_md5_hash);
    fprintf(f, "player name: %s\n", r->player_name);
    fprintf(f, "replay md5 hash: %s\n", r->replay_md5_hash);

    fprintf(f, "300: %hu\n", r->_300);
    fprintf(f, "100: %hu\n", r->_100);
    fprintf(f, "50: %hu\n", r->_50);
    fprintf(f, "geki: %hu\n", r->_geki);
    fprintf(f, "katu: %hu\n", r->_katu);
    fprintf(f, "miss: %hu\n", r->_miss);

    fprintf(f, "score: %u\n", r->score);
    fprintf(f, "max combo: %hu\n", r->max_combo);
    fprintf(f, "Full combo: %hhu\n", r->fc);
    fputs("mods: ", f);  mod_print(f, r->mods); fputs("\n", f);  

    fprintf(f, "lifebar_graph: %s\n", r->lifebar_graph);

    
    fputs("date: ", f);//  %lu\n", r->timestamp);
    cs_timestamp_string(r->timestamp);
    fputs("\n", f);
    /* fprintf(f, "Time: "); */
    /* fprintf(f, "timestamp: %s\n", s); */
    
    fprintf(f, "replay length: %u bytes\n", r->replay_length);
    fprintf(f, "\n");
    fprintf(f, "__ DATA __ :\n");

    for (int i = 0; i < r->repdata_count; ++i) {
    	struct replay_data *rd = &r->repdata[i];
    	fprintf(f, "%ld|", rd->previous_time);
    	fprintf(f, "%g|", rd->x);
    	fprintf(f, "%g|", rd->y);
    	fprintf(f, "%u\n", rd->keys);
    }

}

void replay_free(struct replay *r)
{
    free(r->bm_md5_hash);
    free(r->player_name);
    free(r->replay_md5_hash);
    free(r->lifebar_graph);
    free(r->repdata);
    free(r);
}


