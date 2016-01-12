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

#ifndef REPLAY_H
#define REPLAY_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>

struct replay_data {
    int64_t previous_time;
    double x;
    double y;
    uint32_t keys;
};

struct replay_life {
    uint64_t time_offset;
    double life_amount;
};

struct replay {
    uint8_t game_mode;
    uint32_t game_version;
    char *bm_md5_hash;
    char *player_name;
    char *replay_md5_hash;
    
    uint16_t _300; // [ taiko: 100%, great, 300pts], mania: 300(not MAX)
    uint16_t _100;  // [taiko: 50%, good, 150pts] , mania: 200
    uint16_t _50;   // ctb: small fruits
    uint16_t _geki; // taiko: big great (100%), mania: MAX
    uint16_t _katu; //taiko: big good (50%), mania 100
    uint16_t _miss;

    uint32_t score;
    uint16_t max_combo; // max combo of score, not map
    
    uint8_t fc;
    uint32_t mods;  // this is a bitfield, see general/mods.h

    /* list of time|life separated by commas,  where time is in milliseconds
       and life is a percentage

       this represent the graph of life during the play  */
    char *lifebar_graph;
    uint32_t replife_size;
    struct replay_life *replife;

    /* the date and time of the play  */
    time_t timestamp;

    /* number of bytes in the compressed lzma stream */
    uint32_t replay_length;
    uint64_t repdata_count;
    struct replay_data *repdata;
};

struct replay *replay_parse(FILE *f);
void replay_print(FILE *f, const struct replay *r);
void replay_free(struct replay *r);

#endif //REPLAY_H
