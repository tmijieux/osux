/*
 *  Copyright (©) 2015-2016 Lucas Maugère, Thomas Mijieux
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
#ifndef TAIKO_RANKING_SCORE_H
#define TAIKO_RANKING_SCORE_H

struct tr_map;

struct tr_score
{
    // if need to restart
    const struct tr_map *origin; // map 100% acc

    // values to get at the end
    double acc; // acc
    int great;
    int good;
    int miss;

    double last_point;
    double step;

    // working:
    struct tr_map *map; // current map

    void (*trs_prepare)(struct tr_score *);
    int (*trs_has_reached_step)(struct tr_score *);
};

void trs_main(const struct tr_map *map);
void trs_main_replay(char *replay_file_name, struct tr_map *map);
void trs_print(const struct tr_score *score);

#endif //TAIKO_RANKING_SCORE_H
