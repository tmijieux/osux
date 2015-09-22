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

#include "replay.h"
#include <stdlib.h>
#include <stdint.h>

#include <map/map.h> // game modes
#include <unistd.h>
#include <util/uleb128/uleb128.h>
#include <util/list/list.h>
#include <util/string/split.h>

struct replay_data {
    int64_t previous_time;
    double x;
    double y;
    uint32_t keys;
};

struct replay {
    uint8_t game_mode;
    uint32_t game_version;
    char *bm_md5_hash;
    char *player_name;
    char *replay_md5_hash;
    
    uint16_t _300;
    uint16_t _100;
    uint16_t _50;
    uint16_t _geki;
    uint16_t _katu;
    uint16_t _miss;

    uint32_t score;
    uint16_t max_combo;
    uint8_t fc;
    uint32_t mods;
    char *xy;
    uint64_t timestamp;
    uint32_t replay_length;

    size_t rdc;
    struct replay_data *rd;
};


void lzma_decompress(FILE *f, uint8_t **buf);

static unsigned int parse_replay_data(FILE *f, struct replay_data **rd)
{
    char *uncomp;
    lzma_decompress(f, (uint8_t**) &uncomp);
    char **tab;
    unsigned int rdc = string_split(uncomp, ",", &tab);
    free(uncomp);
    *rd = calloc(sizeof(**rd), rdc);
    for (int i = 0; i < rdc; ++i) {
	sscanf(tab[i], "%ld|%lg|%lg|%d",
	       &(*rd)[i].previous_time,
	       &(*rd)[i].x,
	       &(*rd)[i].y,
    	       &(*rd)[i].keys);
	free(tab[i]);
    }
    free(tab);
    return rdc;
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

    read_string(&r->xy, f);
    
    fread(&r->timestamp, 8, 1, f);
    fread(&r->replay_length, 4, 1, f);

    r->rdc = parse_replay_data(f, &r->rd);
    
    return r;
}

int cs_timestamp_string(uint64_t timestamp);

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
    fprintf(f, "mods: %u\n", r->mods);
    fprintf(f, "xy: %s\n", r->xy);

    
    fprintf(f, "timestamp: %lu\n", r->timestamp);
    /* cs_timestamp_string(r->timestamp); */
    /* fprintf(f, "Time: "); */
    /* fprintf(f, "timestamp: %s\n", s); */

    
    fprintf(f, "replay length: %u\n", r->replay_length);

    fprintf(f, "\n");
    fprintf(f, "__ DATA __ :\n");

    for (int i = 0; i < r->rdc; ++i) {
    	struct replay_data *rd = &r->rd[i];
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
    free(r->xy);
    free(r->rd);
    free(r);
}
