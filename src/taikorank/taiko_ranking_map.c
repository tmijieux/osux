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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osux.h"

#include "bpm.h"
#include "taiko_ranking_object.h"
#include "taiko_ranking_map.h"
#include "print.h"
#include "tr_db.h"
#include "tr_mods.h"
#include "compute_stars.h"

#include "config.h"
#include "check_osu_file.h"

#define BASIC_SV 1.4

#define TYPE(type)    (type & (~HITOBJECT_NEWCOMBO) & 0x0F)
// this get rid of the 'new_combo' flag to get the hit object's type
// more easily

static int convert_get_type(osux_hitobject * ho);
static double convert_get_bpm_app(osux_timingpoint * tp,
				  double sv);
static int convert_get_end_offset(osux_hitobject * ho, int type,
				  double bpm_app);
static struct tr_map * trm_convert(char* file_name);
static void trm_add_to_ps(struct tr_map * map,
			  enum played_state ps, int i);

static void trm_acc(struct tr_map * map);
static struct tr_map * trm_convert_map(osux_beatmap * map);
static struct tr_map * trm_convert(char * filename);

//--------------------------------------------------

void trm_main(const struct tr_map * map)
{
    struct tr_map * map_copy = trm_copy(map);
    trm_set_mods(map_copy, map->conf->mods);
    trm_set_read_only_objects(map_copy);

    // modifications
    trm_add_modifier(map_copy);

    // compute
    trm_apply_mods(map_copy);
    trm_compute_stars(map_copy);

    // printing
    #pragma omp critical
    if(OPT_PRINT_YAML)
	trm_print_yaml(map_copy);
    else {
	if(OPT_PRINT_TRO)
	    trm_print_tro(map_copy, OPT_PRINT_FILTER);
	trm_print(map_copy);
    }

    // db
    if(OPT_DATABASE)
	trm_db_insert(map_copy);

    // free
    trm_free(map_copy);
}

//--------------------------------------------------

void trm_set_read_only_objects(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	map->object[i].objs = map->object;
}

//--------------------------------------------------

void trm_add_modifier(struct tr_map * map)
{
    if(OPT_FLAT)
	trm_flat_big(map);
    if(OPT_NO_BONUS)
	trm_remove_bonus(map);
}

//--------------------------------------------------

void trm_set_mods(struct tr_map * map, int mods)
{
    map->mods = mods;
}

//--------------------------------------------------

struct tr_map * trm_copy(const struct tr_map * map)
{
    struct tr_map * copy = calloc(sizeof(*copy), 1);
    memcpy(copy, map, sizeof(*map));

    copy->object = tro_copy(map->object, map->nb_object);

    copy->title   = strdup(map->title);
    copy->artist  = strdup(map->artist);
    copy->source  = strdup(map->source);
    copy->creator = strdup(map->creator);
    copy->diff    = strdup(map->diff);
    copy->title_uni  = strdup(map->title_uni);
    copy->artist_uni = strdup(map->artist_uni);
    copy->hash = strdup(map->hash);

    return copy;
}

//-----------------------------------------------------

void trm_free(struct tr_map * map)
{
    if(map == NULL)
	return;

    free(map->title);
    free(map->artist);
    free(map->source);
    free(map->creator);
    free(map->diff);
    free(map->title_uni);
    free(map->artist_uni);
    free(map->hash);

    free(map->object);
    free(map);
}

//--------------------------------------------------

struct tr_map *trm_new(char *filename)
{
    struct tr_map *res = NULL;
    char *path = NULL;

    switch ( tr_check_file(filename) ) {
    case TR_FILENAME_OSU_FILE:
	res = trm_convert(filename);
        break;
    case TR_FILENAME_HASH:
	if (ODB = NULL) break;
        path = osux_db_get_beatmap_path_by_hash(ODB, filename);
        if (path == NULL) break;
        res = trm_convert(path);
        break;
    default:
    case TR_FILENAME_ERROR:
        tr_error("Could not load: '%s'", filename);
        break;
    }

    free(path);
    return res;
}

//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------

static int convert_get_type(osux_hitobject * ho)
{
    int bits = 0;
    int sample = ho->hitsound.sample;
    
    if(HIT_OBJECT_IS_SLIDER(ho))
	bits |= TRO_R;
    else if(HIT_OBJECT_IS_SPINNER(ho))
	bits |= TRO_S;
    else if(HIT_OBJECT_IS_CIRCLE(ho)) {
	if((sample & (SAMPLE_WHISTLE | SAMPLE_CLAP)) != 0)
	    bits |= TRO_K;
	else
	    bits |= TRO_D;
    }
    if((sample & SAMPLE_FINISH) != 0)
	return bits | TRO_BIG;
    else
	return bits;
}

//---------------------------------------------------------------

static double convert_get_bpm_app(osux_timingpoint *tp, double sv)
{
    double sv_multiplication;

    if (!tp->inherited)
	sv_multiplication = 1.;
    else
	sv_multiplication = -100. / tp->slider_velocity_multiplier;

    return (mpb_to_bpm(tp->millisecond_per_beat) *
            sv_multiplication * (sv / BASIC_SV));
}

//---------------------------------------------------------------

static int convert_get_end_offset(
    osux_hitobject * ho, int type, double bpm_app)
{
    if (type & TRO_S) {
	return ho->spinner.end_offset;
    }
    if (type & TRO_R) {
	return ho->offset + ((ho->slider.length * ho->slider.repeat) *
			     (MSEC_IN_MINUTE / (100. * BASIC_SV)) /
			     bpm_app);
    }
    // else circle
    return ho->offset;
}

//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------

static struct tr_map *trm_convert(char *filename)
{
    osux_beatmap map;
    if (osux_beatmap_init(&map, filename) < 0) {
        osux_error("Cannot open beatmap %s\n", filename);
        exit(EXIT_FAILURE);
    }
    struct tr_map *res = trm_convert_map(&map);
    osux_beatmap_free(&map);
    return res;
}

//---------------------------------------------------------------

static int trm_convert_map_prepare(osux_beatmap *map)
{
    switch (map->game_mode) {
    case GAME_MODE_STD:
	if (OPT_AUTOCONVERT) {
	    if (osux_beatmap_taiko_autoconvert(map) == 0)
                return 0;
	    tr_error("autoconversion error.");
	} else
	    tr_error("autoconverting map from std game mode is disabled...");
        return -1;
        
    case GAME_MODE_TAIKO:
	return 0;

    case GAME_MODE_CTB:
	tr_error("Catch the beat?!");
        return -1;
    case GAME_MODE_MANIA:
        tr_error("Taiko is not 2k."); // lol
        return -1;
    default:
        break;
    }
    return -1;
}

//---------------------------------------------------------------

static struct tr_map * trm_convert_map(osux_beatmap *map)
{
    if (trm_convert_map_prepare(map) < 0)
        return NULL;

    struct tr_map * tr_map = calloc(sizeof(struct tr_map), 1);
    tr_map->nb_object = map->hitobject_count;
    tr_map->object = calloc(sizeof(struct tr_object), map->hitobject_count);

    // set objects
    unsigned int current_tp = 0;
    tr_map->max_combo = 0;
    for(unsigned int i = 0; i < map->hitobject_count; i++) {
	while(current_tp < (map->timingpoint_count - 1) &&
	      map->timingpoints[current_tp + 1].offset
	      <= map->hitobjects[i].offset)
	    current_tp++;

	tr_map->object[i].offset     =
	    (int) map->hitobjects[i].offset;
	tr_map->object[i].bf         =
	    convert_get_type(&map->hitobjects[i]);
	tr_map->object[i].bpm_app    =
	    convert_get_bpm_app(&map->timingpoints[current_tp],
				map->SliderMultiplier);
	tr_map->object[i].end_offset =
	    convert_get_end_offset(&map->hitobjects[i],
				   tr_map->object[i].bf,
				   tr_map->object[i].bpm_app);

	if(tro_is_bonus(&tr_map->object[i])) {
	    tr_map->object[i].ps = BONUS;
	} else {
	    tr_map->max_combo++;
	    tr_map->object[i].ps = GREAT;
	}
	tr_map->object[i].objs = NULL;
    }

    // get other data
    tr_map->hash = map->md5_hash;
    tr_map->od = map->OverallDifficulty;
    tr_map->title      = strdup(map->Title);
    tr_map->artist     = strdup(map->Artist);
    tr_map->source     = strdup(map->Source);
    tr_map->creator    = strdup(map->Creator);
    tr_map->diff       = strdup(map->Version);
    tr_map->bms_osu_ID  = map->BeatmapSetID;
    tr_map->diff_osu_ID = map->BeatmapID;
    if(map->TitleUnicode == NULL)
	tr_map->title_uni = strdup(tr_map->title);
    else
	tr_map->title_uni  = strdup(map->TitleUnicode);
    if(map->ArtistUnicode == NULL)
	tr_map->artist_uni = strdup(tr_map->artist);
    else
	tr_map->artist_uni = strdup(map->ArtistUnicode);
    tr_map->great = tr_map->max_combo;
    tr_map->good  = 0;
    tr_map->miss  = 0;
    tr_map->bonus = tr_map->nb_object - tr_map->max_combo;
    if(tr_map->max_combo != 0)
	tr_map->acc = MAX_ACC;
    else
	tr_map->acc = 0;

    return tr_map;
}

//---------------------------------------------------------------
//---------------------------------------------------------------
//---------------------------------------------------------------

void trm_print_tro(struct tr_map * map, int filter)
{
    if((filter & FILTER_BASIC) != 0)
	fprintf(OUTPUT_INFO, "offset\trest\ttype\tbpm app\tstate\t");
    if((filter & FILTER_BASIC_PLUS) != 0)
	fprintf(OUTPUT_INFO, "offset\tend\trest\ttype\tbpm app\tstate\t");
    if((filter & FILTER_ADDITIONNAL) != 0)
	fprintf(OUTPUT_INFO, "l hand\tr hand\tobj app\tobj dis\t");
    if((filter & FILTER_DENSITY) != 0)
	fprintf(OUTPUT_INFO, "dnst rw\tdnst cl\tdnst*\t");
    if((filter & FILTER_READING) != 0)
	fprintf(OUTPUT_INFO, "app\tdis\tseen\tread*\t");
    if((filter & FILTER_READING_PLUS) != 0)
	fprintf(OUTPUT_INFO, "app\tend app\tdis\tend dis\tenddis2\tline_a\tb\tb_end\tseen\tread*\t");
    if((filter & FILTER_ACCURACY) != 0)
	fprintf(OUTPUT_INFO, "slow\thitwin\tspc\tacc*\t");
    if((filter & FILTER_PATTERN) != 0)
	fprintf(OUTPUT_INFO, "proba\tpattrn\tpttrn*\t");
    if((filter & FILTER_STAR) != 0)
	fprintf(OUTPUT_INFO, "dst*\tread*\tptrn*\tacc*\tfin*\t");

    fprintf(OUTPUT_INFO, "\n");

    for(int i = 0; i < map->nb_object; ++i)
	tro_print(&map->object[i], filter);
}

//-------------------------------------------------

#define CASE_PRINT(C, STAR)			\
    case C:					\
    fprintf(OUTPUT, "%.4g\t", STAR);		\
    break

void trm_print(struct tr_map * map)
{
    char * order = OPT_PRINT_ORDER;
    int i = 0;
    while(order[i]) {
	switch(order[i]) {
	    CASE_PRINT('F', map->final_star);
	    CASE_PRINT('D', map->density_star);
	    CASE_PRINT('R', map->reading_star);
	    CASE_PRINT('P', map->pattern_star);
	    CASE_PRINT('A', map->accuracy_star);
	default:
	    break;
	}
	i++;
    }
    fprintf(OUTPUT, "(%.4g%%)\t", map->acc * COEFF_MAX_ACC);
    trm_print_mods(map);
    print_string_size(map->diff,    24, OUTPUT);
    print_string_size(map->title,   32, OUTPUT);
    print_string_size(map->creator, 16, OUTPUT);
    fprintf(OUTPUT, "\n");
}

//--------------------------------------------------

void tr_print_yaml_init(void)
{
    fprintf(OUTPUT, "maps: [");
}

void tr_print_yaml_exit(void)
{
    if(OPT_PRINT_YAML)
	fprintf(OUTPUT, "]\n");
}

void trm_print_yaml(struct tr_map * map)
{
    static char * prefix = "";
    if(prefix[0] == 0)
	tr_print_yaml_init();

    char * mods = trm_mods_to_str(map);

    fprintf(OUTPUT, "%s{", prefix);
    fprintf(OUTPUT, "title: \"%s\", ", map->title);
    fprintf(OUTPUT, "title_uni: \"%s\", ", map->title_uni);
    fprintf(OUTPUT, "artist: \"%s\", ", map->artist);
    fprintf(OUTPUT, "artist_uni: \"%s\", ", map->artist_uni);
    fprintf(OUTPUT, "source: \"%s\", ", map->source);
    fprintf(OUTPUT, "creator: \"%s\", ", map->creator);
    fprintf(OUTPUT, "difficulty: \"%s\", ", map->diff);

    fprintf(OUTPUT, "accuracy: %g, ", map->acc * COEFF_MAX_ACC);
    fprintf(OUTPUT, "great: %d, ",  map->great);
    fprintf(OUTPUT, "good: %d, ",  map->good);
    fprintf(OUTPUT, "miss: %d, ",  map->miss);
    fprintf(OUTPUT, "bonus: %d, ", map->bonus);

    fprintf(OUTPUT, "max_combo: %d, ", map->max_combo);
    fprintf(OUTPUT, "combo: %d, ", map->combo);

    fprintf(OUTPUT, "mods: \"%s\", ", mods);

    fprintf(OUTPUT, "stars: {");
    fprintf(OUTPUT, "density_star: %g, ", map->density_star);
    fprintf(OUTPUT, "pattern_star: %g, ", map->pattern_star);
    fprintf(OUTPUT, "reading_star: %g, ", map->reading_star);
    fprintf(OUTPUT, "accuracy_star: %g, ", map->accuracy_star);
    fprintf(OUTPUT, "final_star: %g", map->final_star);
    fprintf(OUTPUT, "}");

    if(OPT_PRINT_TRO) {
	fprintf(OUTPUT, ", objects: [");
	for(int i = 0; i < map->nb_object; i++) {
	    tro_print_yaml(&map->object[i]);
	    fprintf(OUTPUT, ", ");
	}
	fprintf(OUTPUT, "]");
    }

    fprintf(OUTPUT, "}");
    free(mods);
    prefix = ", ";
}

//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------

int trm_hardest_tro(struct tr_map * map)
{
    int best = 0;
    for(int i = 0; i < map->nb_object; i++)
	if(map->object[i].final_star >= map->object[best].final_star &&
	   map->object[i].ps == GREAT)
	    best = i;
    return best;
}

//--------------------------------------------------

int trm_best_influence_tro(struct tr_map * map)
{
    int best = -1;
    double star = map->final_star;
    for(int i = 0; i < map->nb_object; i++) {
	if(map->object[i].ps != GREAT)
	    continue;
	struct tr_map * map_copy = trm_copy(map);
	trm_set_tro_ps(map_copy, i, MISS);
	trm_compute_stars(map_copy);
	if(star > map_copy->final_star) {
	    best = i;
	    star = map_copy->final_star;
	}
	trm_free(map_copy);
    }
    return best;
}

//--------------------------------------------------

static void trm_add_to_ps(struct tr_map * map,
			  enum played_state ps, int i)
{
    switch(ps) {
    case GREAT:
	map->great += i;
	break;
    case GOOD:
	map->good += i;
	break;
    case MISS:
	map->miss += i;
	break;
    case BONUS:
	tr_error("Cannot change bonus!");
	break;
    }
}

//--------------------------------------------------

void trm_set_tro_ps(struct tr_map * map, int x, enum played_state ps)
{
    if(map->object[x].ps == ps)
	tr_error("Object is already with the played state wanted.");
    trm_add_to_ps(map, map->object[x].ps, -1);
    trm_add_to_ps(map, ps, 1);
    map->object[x].ps = ps;
    trm_acc(map);
    if(ps == GOOD) {
	map->object[x].density_star = 0;
	map->object[x].reading_star = 0;
	map->object[x].pattern_star = 0;
	map->object[x].accuracy_star = 0;
	map->object[x].final_star = 0;
    }
}

//--------------------------------------------------

double compute_acc(int great, int good, int miss)
{
    return (((double)(great + good * 0.5)) / (great + good + miss));
}

static void trm_acc(struct tr_map * map)
{
    map->acc = compute_acc(map->great, map->good, map->miss);
}

//--------------------------------------------------

void trm_flat_big(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++) {
	map->object[i].bf &= ~TRO_BIG; // remove big field
    }
}

void trm_remove_tro(struct tr_map * map, int o)
{
    for(int i = o; i < map->nb_object - 1; i++)
	map->object[i] = map->object[i+1];
    map->nb_object--;
}

void trm_remove_bonus(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	if(tro_is_bonus(&map->object[i]))
	    trm_remove_tro(map, i);
}
