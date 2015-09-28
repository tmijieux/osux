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

#include "storyboard.h"
#include "general/game_mode.h"


struct map {
    uint32_t version;
    uint32_t bom;
    
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


extern struct map DEFAULT_MAP;
void map_print(const struct map *m, FILE *f);
void map_free(struct map*);

#endif //MAP_H
