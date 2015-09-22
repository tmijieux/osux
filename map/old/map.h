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

#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <stdint.h>

#define MODE_STD     0
#define MODE_TAIKO   1
#define MODE_CTB     2
#define MODE_MANIA   3

struct map {
    uint32_t version;
    uint32_t bom;
    
    // general info
    char *audio_filename;
    uint32_t audio_lead_in;
    uint32_t preview_time;
    uint32_t count_down;
    char *sample_type;
    uint32_t sample_type_i;
    double stack_leniency;
    uint32_t mode;
    uint32_t letterbox_in_breaks;
    uint32_t widescreen_storyboard;

    // Editor settings
    uint32_t bkmkc;  uint32_t *bookmarks;
    double distance_spacing;
    uint32_t beat_divisor;
    uint32_t grid_size;
    double timeline_zoom;

    // Metadata
    char *title;
    char *title_unicode;
    char *artist;
    char *artist_unicode;
    char *creator;
    char *diff_version; // difficulty name
    char *source;
    uint32_t tagc;  char **tags;
    uint32_t beatmap_id;
    uint32_t beatmap_set_id;

    // Difficulty
    double hp_drain_rate;
    double circle_size;
    double overall_difficulty;
    double approach_rate;
    double slider_multiplier;
    double slider_tick_rate;

    //struct event ev;

    uint32_t tpc;  struct timing_point *tpv;
    uint32_t colc; struct color *colv;
    uint32_t hoc;  struct hit_object *hov;
};


extern struct map DEFAULT_MAP;
void map_print(const struct map *m, FILE *f);
void map_free(struct map*);

#endif //MAP_H
