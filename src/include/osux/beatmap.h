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

#ifndef osux_BEATMAP_H
#define osux_BEATMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <openssl/md5.h>

#include "osux/storyboard.h"
#include "osux/game_mode.h"

typedef struct osux_beatmap osux_beatmap;

struct osux_beatmap {
    uint32_t beatmap_id;
    uint32_t beatmap_set_id;

    uint32_t osu_forum_thrd;

    char *md5_hash;
    char *osu_filename;
    char *path;

    time_t last_modification;
    time_t last_checked;

    // stats
    uint16_t circles;
    uint16_t sliders;
    uint16_t spinners;
    uint32_t drain_time;
    uint32_t total_time;
    uint16_t bpm_avg;
    uint16_t bpm_max;
    uint16_t bpm_min;
    bool already_played;
    time_t last_played;

    //offsets
    uint16_t local_offset;
    uint16_t online_offset;

    // misc
    bool ignore_hitsound;
    bool ignore_skin;
    bool disable_sb;
    bool disable_video;
    bool visual_override;
    uint8_t mania_scroll_speed;

    // .osu  SECTION :
    uint32_t version;
    uint32_t bom; // byte order mark; utf-8 stuff

    // general info
    char *AudioFilename;
    uint32_t AudioLeadIn;
    uint32_t PreviewTime;
    uint32_t Countdown;
    char *SampleSet;  // sample type !
    double StackLeniency;
    uint32_t Mode;
    uint32_t LetterboxInBreaks;
    uint32_t WidescreenStoryboard;

    // Editor settings
    uint32_t bkmkc;  uint32_t *Bookmarks;
    double DistanceSpacing;
    uint32_t BeatDivisor;
    uint32_t GridSize;
    double TimelineZoom;

    // Metadata
    char *Title;
    char *TitleUnicode;
    char *Artist;
    char *ArtistUnicode;
    char *Creator;
    char *Version; // difficulty name
    char *Source;
    uint32_t tagc;  char **Tags;
    uint32_t BeatmapID;
    uint32_t BeatmapSetID;

    // Difficulty
    double HPDrainRate;
    double CircleSize;
    double OverallDifficulty;
    double ApproachRate;
    double SliderMultiplier;
    double SliderTickRate;

    struct storyboard sb;

    uint32_t tpc;  struct timing_point *TimingPoints;
    uint32_t colc; struct color *Colours;
    uint32_t hoc;  struct hit_object *HitObjects;
};

int osux_beatmap_open(const char *filename, osux_beatmap **beatmap);
int osux_beatmap_reopen(osux_beatmap *bm_in, osux_beatmap **bm_out);
int osux_beatmap_save(const char *filename, const osux_beatmap* beatmap);
int osux_beatmap_close(osux_beatmap *beatmap);
int osux_beatmap_print(const osux_beatmap *m, FILE *f);

#endif //osux_BEATMAP_H
