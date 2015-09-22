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

#include <stdio.h>
#include <stdlib.h>

#include "map.h"
#include "color.h"

#include "ho/hit_object.h"
#include "timing/timing_point.h"



struct map DEFAULT_MAP = {
    .timeline_zoom = 1.
};

void map_print(const struct map *m, FILE *f)
{
    if (m->bom) {
	fprintf(f, "%c%c%c", 0xef, 0xbb, 0xbf);
    }
    fprintf(f, "osu file format v%d\r\n", m->version);
    fputs("\r\n", f);
    
    fputs("[General]\r\n", f);
    fprintf(f, "AudioFilename: %s\r\n", m->audio_filename);
    fprintf(f, "AudioLeadIn: %d\r\n", m->audio_lead_in);
    fprintf(f, "PreviewTime: %d\r\n", m->preview_time);
    fprintf(f, "Countdown: %d\r\n", m->count_down);
    fprintf(f, "SampleSet: %s\r\n",  // THIS IS SAMPLE TYPE! ;(
	    m->sample_type);

    fprintf(f, "StackLeniency: %.15g\r\n", m->stack_leniency);
    fprintf(f, "Mode: %d\r\n", m->mode);
    fprintf(f, "LetterboxInBreaks: %d\r\n", m->letterbox_in_breaks);
    fprintf(f, "WidescreenStoryboard: %d\r\n", m->widescreen_storyboard);
    fputs("\r\n", f);

    fputs("[Editor]\r\n", f);
    if (m->bkmkc) {
	fprintf(f, "Bookmarks: ");
	for (int i = 0; i < m->bkmkc; ++i){
	    if (i>=1) fprintf(f, ",");
	    fprintf(f, "%d", m->bookmarks[i]);
	}
    fputs("\r\n", f);
    }
    fprintf(f, "DistanceSpacing: %.15g\r\n", m->distance_spacing);
    fprintf(f, "BeatDivisor: %d\r\n", m->beat_divisor);
    fprintf(f, "GridSize: %d\r\n", m->grid_size);
    fprintf(f, "TimelineZoom: %.8g\r\n", m->timeline_zoom);
    fputs("\r\n", f);
	
    fputs("[Metadata]\r\n", f);
    fprintf(f, "Title:%s\r\n", m->title);
    fprintf(f, "TitleUnicode:%s\r\n", m->title_unicode);
    fprintf(f, "Artist:%s\r\n", m->artist);
    fprintf(f, "ArtistUnicode:%s\r\n", m->artist_unicode);
    fprintf(f, "Creator:%s\r\n", m->creator); 
    fprintf(f, "Version:%s\r\n", m->diff_version);
    fprintf(f, "Source:%s\r\n", m->source);
    fprintf(f, "Tags:");
    for (int i = 0; i < m->tagc; ++i) {
	fprintf(f, "%s", m->tags[i]);
	if (i < m->tagc-1) fprintf(f, " ");
    }
    fputs("\r\n", f);

    fprintf(f, "BeatmapID:%d\r\n", m->beatmap_id);
    fprintf(f, "BeatmapSetID:%d\r\n", m->beatmap_set_id);
    fputs("\r\n", f);
    
    fputs("[Difficulty]\r\n", f);
    fprintf(f, "HPDrainRate:%g\r\n", m->hp_drain_rate);
    fprintf(f, "CircleSize:%g\r\n", m->circle_size);
    fprintf(f, "OverallDifficulty:%g\r\n", m->overall_difficulty);
    fprintf(f, "ApproachRate:%g\r\n", m->approach_rate);
    fprintf(f, "SliderMultiplier:%.15g\r\n", m->slider_multiplier); 
    fprintf(f, "SliderTickRate:%g\r\n", m->slider_tick_rate);
    fputs("\r\n", f);

    fputs("[Events]\r\n", f);
    fputs("\r\n", f);
    
    fputs("[TimingPoints]\r\n", f);
    for (int i = 0; i < m->tpc; ++i)
	tp_print(&m->tpv[i]);
    fputs("\r\n", f);      

    if (m->colc) {
	fputs("[Colours]\r\n", f);
	for (int i = 0; i < m->colc; ++i)
	    col_print(&m->colv[i], i+1);
	fputs("\r\n", f);
    }
    
    fputs("[HitObjects]\r\n", f);
    for (int i = 0; i < m->hoc; ++i)
	ho_print(&m->hov[i], m->version);
    fputs("\r\n", f);
}


void map_free(struct map *m)
{
    free(m->audio_filename);
    free(m->sample_type);
    free(m->bookmarks);
    free(m->title);
    free(m->title_unicode);
    free(m->artist);
    free(m->artist_unicode);
    free(m->creator);
    free(m->diff_version);
    free(m->source);

    for (int i = 0; i < m->tagc; ++i)
	free(m->tags[i]);
    free(m->tags);

    for (int i = 0; i < m->tpc; ++i)
	tp_free(&m->tpv[i]);
    free(m->tpv);
    free(m->colv);

    for (int i = 0; i < m->hoc; ++i)
	ho_free(&m->hov[i]);

    free(m->hov);

    free(m);
}
