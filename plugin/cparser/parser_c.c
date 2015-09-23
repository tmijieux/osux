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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <string.h>

#include "util/string/split.h"
#include "util/list/list.h"
#include "util/hashtable/hashtable.h"

#include "hs/hitsound.h"
#include "map/map.h"
#include "combocolor/combocolor.h"
#include "ho/hit_object.h"
#include "tp/timing_point.h"

#include "parser_c.h"
#include "c/omp.h"


#define LINE_SIZE  512

#define READ_INT(map, field, ht, entry_id)			\
    ({								\
	char *tmp;						\
	if (ht_get_entry((ht), (entry_id), &tmp) == 0) {	\
	    (map)->field = atoi(tmp);				\
	    free(tmp);						\
	} else {						\
	    (map)->field = DEFAULT_MAP.field;			\
	}							\
    })


#define READ_DOUBLE(map, field, ht, entry_id)			\
    ({								\
	char *tmp;						\
	if (ht_get_entry((ht), (entry_id), &tmp) == 0) {	\
	    (map)->field = atof(tmp);				\
	    free(tmp);						\
	} else {						\
	    (map)->field = DEFAULT_MAP.field;			\
	}							\
    })



static void parse_general(struct hash_table *ht, struct map *m)
{
    // General info
    struct hash_table *g = NULL;
    if (ht_get_entry(ht, "General", &g) != -1) {
	
	ht_get_entry(g, "AudioFilename", &m->audio_filename);
	READ_DOUBLE(m, audio_lead_in,  g, "AudioLeadIn");
	READ_DOUBLE(m, preview_time,   g, "PreviewTime");
	READ_DOUBLE(m, count_down,     g, "Countdown");
	
	ht_get_entry(g, "SampleSet", &m->sample_type);
	switch (m->sample_type[0]) {
	case 'N': m->sample_type_i = HS_NORMAL; break;
	case 'S': m->sample_type_i = HS_SOFT; break;
	case 'D':  m->sample_type[1] == 'e' ? (m->sample_type_i = HS_DEFAULT)
		: (m->sample_type_i = HS_DRUM); break; 
	}
	
	READ_DOUBLE(m, stack_leniency, g, "StackLeniency");
	READ_INT(m, mode,                  g, "Mode");
	READ_INT(m, letterbox_in_breaks,   g, "LetterboxInBreaks");
	READ_INT(m, widescreen_storyboard, g, "WidescreenStoryboard");
	ht_free(g);
    }
}

static void parse_editor(struct hash_table *ht, struct map *m)
{
    // Editor
    struct hash_table *ed = NULL;
    if (ht_get_entry(ht, "Editor", &ed) != -1) {

	char *bkm;
	if (ht_get_entry(ed, "Bookmarks", &bkm) != -1) {
	    char **bk_str;
	    m->bkmkc = string_split(bkm, ",", &bk_str);
	    m->bookmarks = malloc(sizeof(*m->bookmarks) * m->bkmkc);
	    for (int i = 0; i < m->bkmkc; ++i) {
		m->bookmarks[i] = atoi(bk_str[i]);
		free(bk_str[i]);
	    }
	    free(bk_str);
	    free(bkm);
	}
	
	READ_DOUBLE(m, distance_spacing, ed, "DistanceSpacing");
	READ_INT(m, beat_divisor,      ed, "BeatDivisor");
	READ_INT(m, grid_size,         ed, "GridSize");
	READ_DOUBLE(m, timeline_zoom,    ed, "TimelineZoom");
	ht_free(ed);
    }
}

static void parse_metadata(struct hash_table *ht, struct map *m)
{
    struct hash_table *me = NULL;
    if (ht_get_entry(ht, "Metadata", &me) != -1) {
	
	ht_get_entry( me, "Title", &m->title);
	ht_get_entry( me, "TitleUnicode", &m->title_unicode);
	ht_get_entry( me, "Artist", &m->artist);
	ht_get_entry( me, "ArtistUnicode", &m->artist_unicode);
	ht_get_entry( me, "Creator", &m->creator);
	ht_get_entry( me, "Version", &m->diff_version);
	ht_get_entry( me, "Source", &m->source);
	
	char *tags = NULL;
	if (ht_get_entry( me, "Tags", &tags) != -1) {
	    m->tagc = string_split(tags, " ", &m->tags);
	    free(tags);
	}
	
	READ_DOUBLE(m, beatmap_id,      me, "BeatmapID");
	READ_DOUBLE(m, beatmap_set_id,  me, "BeatmapSetID");
    
	ht_free(me);
    }
}

static void parse_colors(struct hash_table *ht, struct map *m)
{
    struct hash_table *c = NULL;
    if (ht_get_entry(ht, "Colours", &c) == -1) {
	m->colc = 0;
	m->colv = NULL;
	return;
    }
    
    int cc, k = 1;
    struct list *coll = list_new(0);
    do {
	char *colval = NULL;
	char colstr[40];
	snprintf(colstr, 40, "Combo%d", k);
	cc = ht_get_entry(c, colstr, &colval);
	struct color *col = malloc(sizeof(*col));

	if (cc != -1) {
	    col_parse(colval, col);
	    list_add(coll, col);
	    free(colval);
	}
	++k;
    } while (cc != -1);
    ht_free(c);
    
    m->colc = list_size(coll);
    m->colv = malloc(sizeof(*m->colv) * m->colc);
    for (int i = 1; i <= m->colc; ++i) {
	void *v = list_get(coll, i);
	m->colv[m->colc - i] = *(struct color*)v;
	free(v); // no colfree
    }
    list_free(coll);
}

static void parse_events(struct hash_table *ht, struct map *m)
{
    struct hash_table *ev = NULL;
    if (ht_get_entry(ht, "Events", &ev) != -1) {
	ht_free(ev);
    }
}

static void parse_hit_objects(struct hash_table *ht, struct map *m)
{
    struct list *HO = NULL;
    ht_get_entry(ht, "HitObjects", &HO);
    m->hoc = list_size(HO);
    m->HitObjects = malloc(sizeof(*m->HitObjects) * m->hoc);
    for (int i = 1 ; i <= m->hoc; ++i) { 
		void * v = list_get(HO, i);
		m->HitObjects[m->hoc - i] = *(struct hit_object*) v;
		free(v); // no ho_free: transferring malloc'd content
    }
    list_free(HO);
}

static void parse_difficulty(struct hash_table *ht, struct map *m)
{

    struct hash_table *d = NULL;
    ht_get_entry(ht, "Difficulty", &d);
    
    READ_DOUBLE(m, d,  HPDrainRate);
    READ_DOUBLE(m, d,  CircleSize);
    READ_DOUBLE(m, d,  OverallDifficulty);
    READ_DOUBLE(m, d,  ApproachRate);
    READ_DOUBLE(m, d,  SliderMultiplier);
    READ_DOUBLE(m, d,  SliderTickRate);

    ht_free(d);
}

static void parse_timing_points(struct hash_table *ht, struct map *m)
{
    struct list *TP;
    ht_get_entry(ht, "TimingPoints", &TP);
    m->tpc = list_size(TP);
    m->TimingPoints = malloc(sizeof(*m->TimingPoints) * m->tpc);
    for (int i = 1 ; i <= m->tpc; ++i) {
	void * v = list_get(TP, i);
	m->TimingPoints[m->tpc - i] = *(struct timing_point*) v;
	free(v); // no ho_free: transferring malloc'd content
    }
    list_free(TP);
}

static void parse(struct hash_table *ht, struct map *m)
{    
    parse_general(ht, m);
    parse_editor(ht, m);
    parse_metadata(ht, m);
    parse_difficulty(ht, m);
    parse_events(ht, m);
    parse_timing_points(ht, m);
    parse_colors(ht, m);
    parse_hit_objects(ht, m);
}

struct map *osux_c_parse_beatmap(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
	perror(filename);
	exit(EXIT_FAILURE);
    }
    unsigned char bom[3];
    int bbom = 1;
    fread(bom, 3, 1, f);
    if ( !(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
	rewind(f);
	bbom = 0;
    }
    
    int v;
    if (fscanf(f, u8"osu file format v%d\r\n", &v) != 1) {
	fprintf(stderr, "\"%s\" seems to be an invalid file!\n", filename);	
	return NULL;
    }

    struct hash_table *sect = omp_c_parse_osu_file(f, v);
    fclose(f);
    
    struct map *m = malloc(sizeof(*m));
    *m = (struct map) { 0 };
    m->version = v;
    m->bom = bbom;
    parse(sect, m);
    ht_free(sect);
    return m;
}
