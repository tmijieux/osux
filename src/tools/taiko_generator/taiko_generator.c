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
#include <string.h>
#include <stdlib.h>

#include "osux.h"
#include "cmdline.h"

struct taiko_generator {
    char *pattern;
    uint32_t len;
    uint32_t pos;

    double offset; // cast to int at the very end
    double offset_unit;
};

static int str_has_char(const char*s, char c)
{
    for (uint32_t i = 0; s[i] != '\0'; i++)
	if (s[i] == c)
	    return 1;
    return 0;
}

static void tg_check_pattern(struct taiko_generator *tg)
{
    static char *allowed = "dDkK_";
    for (uint32_t i = 0; i < tg->len; i++) {
	char c = tg->pattern[i];
	if (!str_has_char(allowed, c)) {
	    fprintf(stderr, "Invalid pattern value '%c'\n", c);
	    exit(EXIT_FAILURE);
	}
    }
}

static struct taiko_generator *tg_new(char *pattern, double bpm)
{
    struct taiko_generator *tg = g_malloc0(sizeof*tg);
    tg->pattern = pattern;
    tg->len = strlen(pattern);
    tg->pos = 0;
    tg->offset = 0;
    tg->offset_unit = (60000. / bpm) / 4.;

    tg_check_pattern(tg);
    return tg;
}

static void tg_free(struct taiko_generator *tg)
{
    g_free(tg);
}

static int tg_get_sample(struct taiko_generator *tg)
{
    switch (tg->pattern[tg->pos]) {
    case 'd':
	return SAMPLE_TAIKO_DON;
    case 'k':
	return SAMPLE_TAIKO_KAT;
    case 'D':
	return SAMPLE_TAIKO_DON | SAMPLE_TAIKO_BIG;
    case 'K':
	return SAMPLE_TAIKO_KAT | SAMPLE_TAIKO_BIG;
    default:
	fprintf(stderr, "Incorrect sample asked.\n");
	return -1;
    }
}

static void tg_next(struct taiko_generator *tg)
{
    tg->pos = (tg->pos + 1) % tg->len;
    tg->offset += tg->offset_unit;
}

static int tg_is_sample(struct taiko_generator *tg)
{
    return tg->pattern[tg->pos] != '_';
}

static void beatmap_set_general(osux_beatmap *bm)
{
    bm->osu_version = 14; // ?
    bm->AudioFilename = strdup("none.mp3");
    bm->Mode = GAME_MODE_TAIKO;
    bm->SampleSet = strdup("");
}

static char *create_title(struct gengetopt_args_info *info)
{
    char *s = "";
    if (info->od_given)
	s = xasprintf("%sOD%.1f - ", s, info->od_arg);
    if (info->svm_given)
	s = xasprintf("%s%dbpma - ", s, (int) (info->svm_arg * info->bpm_arg));
    if (info->bpm_given)
	s = xasprintf("%s%dbpm - ", s, (int) info->bpm_arg);
    if (info->nb_ho_given)
	s = xasprintf("%s%dobj - ", s, info->nb_ho_given);

    if (s[0] == '\0') {
	s = strdup("Default");
    } else {
	uint32_t len = strlen(s);
	s[len-3] = '\0';
    }
    return s;
}

static void beatmap_set_metadata(osux_beatmap *bm,
				 struct gengetopt_args_info *info)
{
    bm->Title         = create_title(info);
    bm->TitleUnicode  = strdup(bm->Title);
    bm->Artist        = strdup(info->artist_arg);
    bm->ArtistUnicode = strdup(bm->Artist);
    bm->Creator       = strdup("Taiko Generator");
    bm->Source        = strdup(bm->Creator);
    bm->Version       = strdup(info->pattern_arg);
    bm->BeatmapID     = -1;
    bm->BeatmapSetID  = -1;
}

static void beatmap_set_difficulty(osux_beatmap *bm,
				   struct gengetopt_args_info *info)
{
    bm->HPDrainRate  = 5;
    bm->CircleSize   = 5;
    bm->ApproachRate = 5;
    bm->OverallDifficulty = info->od_arg;
    bm->SliderMultiplier  = 1.4 * info->svm_arg;
    bm->SliderTickRate = 1;
}

static void beatmap_set_tp(osux_beatmap *bm,
			   struct gengetopt_args_info *info)
{
    osux_timingpoint *tp = g_malloc0(sizeof*tp);
    tp->offset = 0;
    tp->time_signature = 4;
    tp->sample_type = 1;
    tp->volume = 100;
    tp->slider_velocity_multiplier = 1;
    tp->last_non_inherited = tp;
    tp->slider_velocity = bm->SliderMultiplier;
    tp->millisecond_per_beat = 60000. / info->bpm_arg;

    bm->timingpoint_count   = 1;
    bm->timingpoint_bufsize = 1;
    bm->timingpoints = tp;
}

static osux_beatmap *beatmap_new(struct gengetopt_args_info *info)
{
    osux_beatmap *bm = g_malloc0(sizeof*bm);
    beatmap_set_general(bm);
    beatmap_set_metadata(bm, info);
    beatmap_set_difficulty(bm, info);
    beatmap_set_tp(bm, info);
    return bm;
}

static void tg_set_ho(struct taiko_generator *tg, osux_hitobject *ho)
{
    while(!tg_is_sample(tg)) {
	tg_next(tg);
    }
    struct osux_hitobject *tmp = 
	osux_ho_taiko_circle_new(tg->offset, tg_get_sample(tg));
    *ho = *tmp;
    osux_hitobject_free(tmp);
    g_free(tmp);
    tg_next(tg);
}

static osux_beatmap *beatmap_add_ho(osux_beatmap *bm,
				    struct gengetopt_args_info *info)
{
    bm->hitobject_count   = info->nb_ho_arg;
    bm->hitobject_bufsize = info->nb_ho_arg;
    bm->hitobjects = g_malloc0((sizeof*bm->hitobjects) * info->nb_ho_arg);
    struct taiko_generator *tg = tg_new(info->pattern_arg, info->bpm_arg);
    for (int i = 0; i < info->nb_ho_arg; i++) {
	tg_set_ho(tg, &bm->hitobjects[i]);
    }
    tg_free(tg);
    return bm;
}

int main(int argc, char *argv[])
{
    struct gengetopt_args_info info;

    if (cmdline_parser(argc, argv, &info) != 0) {
        fprintf(stderr, "error parsing command line arguments\n");
        exit(EXIT_FAILURE);
    }

    osux_beatmap *bm = beatmap_new(&info);
    beatmap_add_ho(bm, &info);

    char * file = osux_beatmap_default_filename(bm);
    char * path = xasprintf("%s%s", info.output_dir_arg, file);
    int res = osux_beatmap_save(bm, path, false);
    if (res < 0)
	exit(EXIT_FAILURE);
    if (!info.quiet_given)
	printf("Output file: '%s'\n", path);

    free(file);
    free(path);
    osux_beatmap_free(bm);
    g_free(bm);
    cmdline_parser_free(&info);
    return EXIT_SUCCESS;
}
