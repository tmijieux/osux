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

#include "util/split.h"
#include "util/list.h"
#include "util/hash_table.h"

#include "beatmap/hitsound.h"
#include "beatmap/beatmap.h"
#include "beatmap/color.h"
#include "beatmap/hitobject.h"
#include "beatmap/timingpoint.h"

#include "fetch.h"
#include "parse.h"

#define LINE_SIZE  512

#define MAP_READ_INT(map, ht, entry)			\
    ({							\
	char *tmp;					\
	if (ht_get_entry((ht), (#entry), &tmp) == 0) {	\
	    (map)->entry = atoi(tmp);			\
	    free(tmp);					\
	} else {					\
	    (map)->entry = DEFAULT_MAP.entry;		\
	}						\
    })


#define MAP_READ_DOUBLE(map,  ht, entry)		\
    ({							\
	char *tmp;					\
	if (ht_get_entry((ht), (#entry), &tmp) == 0) {	\
	    (map)->entry = atof(tmp);			\
	    free(tmp);					\
	} else {					\
	    (map)->entry = DEFAULT_MAP.entry;		\
	}						\
    })


#define MAP_READ_STRING(map,  ht, entry)		\
    ({							\
	char *tmp;					\
	if (ht_get_entry((ht), (#entry), &tmp) == 0) {	\
	    (map)->entry = tmp;				\
	    free(tmp);					\
	} else {					\
	    (map)->entry = DEFAULT_MAP.entry;		\
	}						\
    })

/******************************************************************************/

static void fetch_general(struct hash_table *ht, struct map *m)
{
    // General info
    struct hash_table *g = NULL;
    if (ht_get_entry(ht, "General", &g) != -1) {
	
	MAP_READ_STRING(m, g, AudioFilename);
	MAP_READ_DOUBLE(m,  g, AudioLeadIn);
	MAP_READ_DOUBLE(m,  g, PreviewTime);
	MAP_READ_DOUBLE(m,  g, Countdown);
	
	ht_get_entry(g, "SampleSet", &m->SampleSet);
	
	MAP_READ_DOUBLE(m, g, StackLeniency);
	MAP_READ_INT(m, g, Mode);
	MAP_READ_INT(m, g, LetterboxInBreaks);
	MAP_READ_INT(m, g, WidescreenStoryboard);
	ht_free(g);
    }
}

static void fetch_editor(struct hash_table *ht, struct map *m)
{
    // Editor
    struct hash_table *ed = NULL;
    if (ht_get_entry(ht, "Editor", &ed) != -1) {

	char *bkm;
	if (ht_get_entry(ed, "Bookmarks", &bkm) != -1) {
	    char **bk_str;
	    m->bkmkc = string_split(bkm, ",", &bk_str);
	    m->Bookmarks = malloc(sizeof(*m->Bookmarks) * m->bkmkc);
	    for (int i = 0; i < m->bkmkc; ++i) {
		m->Bookmarks[i] = atoi(bk_str[i]);
		free(bk_str[i]);
	    }
	    free(bk_str);
	    free(bkm);
	}
	
	MAP_READ_DOUBLE(m, ed, DistanceSpacing);
	MAP_READ_INT(m,    ed, BeatDivisor);
	MAP_READ_INT(m,    ed, GridSize);
	MAP_READ_DOUBLE(m, ed, TimelineZoom);
	ht_free(ed);
    }
}

static void fetch_metadata(struct hash_table *ht, struct map *m)
{
    struct hash_table *me = NULL;
    if (ht_get_entry(ht, "Metadata", &me) != -1) {

	// TODO : USE MAP READ STRING
	ht_get_entry( me, "Title", &m->Title);
	ht_get_entry( me, "TitleUnicode", &m->TitleUnicode);
	ht_get_entry( me, "Artist", &m->Artist);
	ht_get_entry( me, "ArtistUnicode", &m->ArtistUnicode);
	ht_get_entry( me, "Creator", &m->Creator);
	ht_get_entry( me, "Version", &m->Version);
	ht_get_entry( me, "Source", &m->Source);
	
	char *tags = NULL;
	if (ht_get_entry( me, "Tags", &tags) != -1) {
	    m->tagc = string_split(tags, " ", &m->Tags);
	    free(tags);
	}
	
	MAP_READ_DOUBLE(m, me, BeatmapID);
	MAP_READ_DOUBLE(m, me, BeatmapSetID);
    
	ht_free(me);
    }
}


static void fetch_difficulty(struct hash_table *ht, struct map *m)
{

    struct hash_table *d = NULL;
    ht_get_entry(ht, "Difficulty", &d);
    
    MAP_READ_DOUBLE(m, d,  HPDrainRate);
    MAP_READ_DOUBLE(m, d,  CircleSize);
    MAP_READ_DOUBLE(m, d,  OverallDifficulty);
    MAP_READ_DOUBLE(m, d,  ApproachRate);
    MAP_READ_DOUBLE(m, d,  SliderMultiplier);
    MAP_READ_DOUBLE(m, d,  SliderTickRate);

    ht_free(d);
}

/******************************************************************************/

// TODO : remanier ces fonctions

static void col_c_fetch(struct color *c, struct  list *h)
{
    struct list *q = list_get(h, 2);
    
    c->r = * (int*)list_get(q, 1);
    c->g = * (int*)list_get(q, 2);
    c->b = * (int*)list_get(q, 3);
}

static void fetch_colors(struct hash_table *ht, struct map *m)
{

   struct list *color = NULL;
   ht_get_entry(ht, "Colours", &color);
    if (color) {
	int l = list_size(color);
	m->Colours = malloc(sizeof(*m->Colours) * l);
	for (int i = 0; i < l; ++i) {
	    struct list *c = list_get(color, i);
	    col_c_fetch(&m->Colours[i], c);
	}
    }
    
}



/******************************************************************************/

static void fetch_events(struct hash_table *ht, struct map *m)
{
    struct hash_table *ev = NULL;
    if (ht_get_entry(ht, "Events", &ev) != -1) {
	ht_free(ev);
    }
}

/******************************************************************************/

// HITOBJECTS

#define HO_READ_DOUBLE(tp, dict, str)	\
    ({					\
	double *tmp = NULL;		\
	ht_get_entry(dict, #str, &tmp);	\
	if (tmp) {			\
	    tp->str = *tmp;		\
	    free(tmp);			\
	}				\
	else				\
	    tp->str = 0.;		\
    })					\

#define HO_READ_INT(tp, dict, str)	\
    ({					\
	int *tmp = NULL;		\
	ht_get_entry(dict, #str, &tmp);	\
	if (tmp) {			\
	    tp->str = *tmp;		\
	    free(tmp);			\
	}				\
	else				\
	    tp->str = 0.;		\
    })					\


#define HO_READ_STRING(tp, dict, str)	\
    ({					\
	char *tmp;			\
	ht_get_entry(dict, #str, &tmp);	\
	if (tmp)			\
	    tp->str = tmp;		\
	else				\
	    tp->str = NULL;		\
    })					\

static void ho_c_fetch(struct hit_object *ho, struct hash_table *ho_dict)
{
    HO_READ_INT(ho, ho_dict, x);
    HO_READ_INT(ho, ho_dict, y);
    HO_READ_INT(ho, ho_dict, offset);
    HO_READ_INT(ho, ho_dict, type);
    
    HO_READ_INT(ho, ho_dict, hs.sample);
    HO_READ_INT(ho, ho_dict, hs.additional);
    
    HO_READ_INT(ho, ho_dict, hs.st);
    HO_READ_INT(ho, ho_dict, hs.st_additional);
    HO_READ_INT(ho, ho_dict, hs.sample_set_index);
    HO_READ_INT(ho, ho_dict, hs.volume);
    HO_READ_STRING(ho, ho_dict, hs.sfx_filename);

    HO_READ_INT(ho, ho_dict, sli.type); 
    HO_READ_INT(ho, ho_dict, sli.repeat);
    HO_READ_DOUBLE(ho, ho_dict, sli.length);

    // SLIDER POINTS
    struct list *tmp1 = NULL;
    ht_get_entry(ho_dict, "sli.coord", &tmp1);
    if (tmp1) {
	ho->sli.point_count = list_size(tmp1);
	ho->sli.pos = malloc(sizeof(*ho->sli.pos) * ho->sli.point_count);
	for (int i = 0; i < ho->sli.point_count; ++i) {
	    struct list *tmp2 = NULL;
	    list_get(tmp1, i);

	    long *tmp = list_get(tmp2, 0);
	    ho->sli.pos[i].x = *tmp; free(tmp);
	    tmp = list_get(tmp2, 1);
	    ho->sli.pos[i].y = *tmp; free(tmp);
	}
    }

    // SLIDER ADDITIONAL
    HO_READ_INT(ho, ho_dict, sli.hs.additional);
    if (ho->sli.hs.additional) {
	struct list *slisample = NULL;
	ht_get_entry(ho_dict, "sli.hs.sample", &slisample);
	struct list *slihsadd = NULL;
	ht_get_entry(ho_dict, "sli.hs.st_add", &slihsadd);

	ho->sli.hs.dat = malloc(sizeof(*ho->sli.hs.dat) *
				(ho->sli.repeat + 1));
	if (slisample) {
	    for (int i = 0; i < (ho->sli.repeat + 1); ++i) {
		long *tmp = list_get(slisample, i);
		ho->sli.hs.dat[i].sample = *tmp;
		free(tmp);
	    }
	}
	if (slihsadd) { // TODO: disable this for v9 by example check version
	    for (int i = 0; i < (ho->sli.repeat + 1); ++i) {
		struct list *tmp = list_get(slihsadd, i);
		ho->sli.hs.dat[i].st = 
		    PyInt_AsLong(list_get(tmp, 0));
		ho->sli.hs.dat[i].st_additional = 
		    PyInt_AsLong(list_get(tmp, 1));
	    }
	}
    }

    // spinner
    HO_READ_INT(ho, ho_dict, spi.end_offset);
}

static void fetch_hit_objects(struct hash_table *ht, struct map *m)
{
    struct list *HO = NULL;
    ht_get_entry(ht, "HitObjects", &HO);
    m->hoc = list_size(HO);
    m->HitObjects = malloc(sizeof(*m->HitObjects) * m->hoc);
    for (int i = 1 ; i <= m->hoc; ++i) { 
	struct hash_table *h = list_get(HO, i);
	ho_c_fetch(&m->HitObjects[m->hoc - i], h);
	ht_free(h); // no ho_free: transferring malloc'd content
    }
    list_free(HO);
}


/******************************************************************************/

// TIMING POINTS

#define TP_READ_DOUBLE(tp, dict, str)	\
    ({					\
	double *tmp = NULL;		\
	ht_get_entry(dict, #str, &tmp);	\
	if (tmp) {			\
	    tp->str = *tmp;		\
	    free(tmp);			\
	}				\
	else				\
	    tp->str = 0.;		\
    })					\


#define TP_READ_INT(tp, dict, str)	\
    ({					\
	int *tmp = NULL;		\
	ht_get_entry(dict, #str, &tmp);	\
	if (tmp) {			\
	    tp->str = *tmp;		\
	    free(tmp);			\
	}				\
	else				\
	    tp->str = 0.;		\
    })					\

static void tp_c_fetch(struct timing_point *tp, struct hash_table *tp_dict)
{
    TP_READ_DOUBLE(tp, tp_dict, offset);
    TP_READ_DOUBLE(tp, tp_dict, mpb);
    TP_READ_INT(tp, tp_dict, time_signature);
    TP_READ_INT(tp, tp_dict, sample_type);
    TP_READ_INT(tp, tp_dict, sample_set);
    TP_READ_INT(tp, tp_dict, volume);
    TP_READ_INT(tp, tp_dict, uninherited);
    TP_READ_INT(tp, tp_dict, kiai);
}

static void fetch_timing_points(struct hash_table *ht, struct map *m)
{
    struct list *TP;
    ht_get_entry(ht, "TimingPoints", &TP);
    m->tpc = list_size(TP);
    m->TimingPoints = malloc(sizeof(*m->TimingPoints) * m->tpc);
    for (int i = 1 ; i <= m->tpc; ++i) {
	struct hash_table * h = list_get(TP, i);
	tp_c_fetch(&m->TimingPoints[m->tpc - i], h);
	ht_free(h); // no ho_free: transferring malloc'd content
    }
    list_free(TP);
}

/******************************************************************************/

// GLOBAL 
static struct map *fetch_beatmap(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
	perror(filename);
	exit(EXIT_FAILURE);
    }

    struct hash_table *ht = cparse_osu_file(f);
    fclose(f);
    if (!ht) {
	fprintf(stderr, "\"%s\" seems to be an invalid file!\n", filename);
	return NULL;
    }
    
    struct map *m = malloc(sizeof(*m));
    *m = (struct map) { 0 };
    
    MAP_READ_INT(m, ht, version);
    MAP_READ_INT(m, ht, bom);

    fetch_general(ht, m);
    fetch_editor(ht, m);
    fetch_metadata(ht, m);
    fetch_difficulty(ht, m);
    fetch_events(ht, m);
    fetch_timing_points(ht, m);
    fetch_colors(ht, m);
    fetch_hit_objects(ht, m);

     ht_free(ht);
    return m;
}



/******************************************************************************/


__export
struct map *osux_c_parse_beatmap(const char *filename)
{
    return fetch_beatmap(filename);
}
