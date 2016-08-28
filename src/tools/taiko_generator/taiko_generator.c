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
#include <limits.h>

#include "osux.h"
#include "cmdline.h"

struct taiko_generator {
    char *pattern;
    uint32_t len;
    uint32_t pos;

    int32_t randomness;
    GRand *rand;

    double offset; // cast to int at the very end
    double offset_unit;
};

static uint32_t rotl32(uint32_t value, unsigned int count) {
    const unsigned int mask = (CHAR_BIT * sizeof(value) - 1);
    count &= mask;
    return (value << count) | (value >> ((-count) & mask));
}

static uint32_t rotr32(uint32_t value, unsigned int count) {
    const unsigned int mask = (CHAR_BIT * sizeof(value) - 1);
    count &= mask;
    return (value >> count) | (value << ((-count) & mask));
}

static int
make_taiko_circle(osux_hitobject *ho, int offset, int sample)
{
    int err = 0;
    char *rep = NULL;
    rep = g_strdup_printf(
        "0,0,%d,%d,%d,0:0:0:0:", offset, HITOBJECT_CIRCLE, sample);
    err = osux_hitobject_init(ho, rep, 12);
    g_free(rep);
    return err;
}

static int
make_taiko_spinner(osux_hitobject *ho, int offset, int end_offset)
{
    int err = 0;
    char *rep = NULL;
    rep = g_strdup_printf(
        "0,0,%d,%d,%d,%d,0:0:0:0:", offset,
        HITOBJECT_SPINNER | HITOBJECT_NEWCOMBO,  SAMPLE_NORMAL, end_offset);
    err = osux_hitobject_init(ho, rep, 12);
    g_free(rep);
    return err;
}

/*
 * Create a seed from 'useful' parameters.
 * Thus, maps with the same parameters will be the same.
 */
static uint32_t tg_seed(const struct gengetopt_args_info *info)
{
    uint32_t seed = info->random_arg * 10;
    for (uint32_t i = 0; info->pattern_arg[i] != '\0'; i++)
	seed += info->pattern_arg[i] * (5 + i);
    seed ^= rotl32(info->nb_ho_arg, 10);
    seed -= info->bpm_arg * 3;
    seed += rotl32(info->abpm_arg / info->bpm_arg, 8);
    seed ^= rotr32(info->od_arg, 3);
    return seed;
}

static int str_has_char(const char *s, char c)
{
    for (uint32_t i = 0; s[i] != '\0'; i++)
	if (s[i] == c)
	    return 1;
    return 0;
}

static void tg_check_pattern(const struct taiko_generator *tg)
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

static int tg_init(struct taiko_generator *tg, char *pattern, double bpm, int randomness, uint32_t seed)
{
    memset(tg, 0, sizeof*tg);
    tg->pattern = pattern;
    tg->len = strlen(pattern);
    tg->randomness = randomness;
    tg->rand = g_rand_new_with_seed(seed);
    tg->pos = 0;
    tg->offset = 0;
    tg->offset_unit = (60000. / bpm) / 4.;
    tg_check_pattern(tg);
    return 0;
}

static int tg_free(struct taiko_generator *tg)
{
    g_rand_free(tg->rand);
    return 0;
}

static int32_t tg_rand(const struct taiko_generator *tg)
{
    if (tg->randomness)
	return g_rand_int_range(tg->rand, -tg->randomness, tg->randomness);
    return 0;
}

static int tg_get_sample(const struct taiko_generator *tg)
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

static int tg_is_blank(const struct taiko_generator *tg)
{
    return tg->pattern[tg->pos] == '_';
}

static void beatmap_set_general(osux_beatmap *bm)
{
    bm->osu_version = 14; // ?
    bm->AudioFilename = strdup("none.mp3");
    bm->Mode = GAME_MODE_TAIKO;
    bm->SampleSet = SAMPLE_TYPE_NORMAL;
}

static char *create_title(struct gengetopt_args_info *info)
{
    char *s = "";
    if (info->od_given)
	s = xasprintf("%sOD%.1f - ", s, info->od_arg);
    if (info->abpm_given)
	s = xasprintf("%s%dabpm - ", s, (int) info->abpm_arg);
    if (info->bpm_given)
	s = xasprintf("%s%dbpm - ", s, (int) info->bpm_arg);
    if (info->nb_ho_given)
	s = xasprintf("%s%dobj - ", s, info->nb_ho_arg);
    if (info->random_given)
	s = xasprintf("%s%drand - ", s, info->random_arg);

    if (s[0] == '\0') {
	s = strdup("Default");
    } else {
	uint32_t len = strlen(s);
	s[len-3] = '\0';
    }
    return s;
}

static void
beatmap_set_metadata(osux_beatmap *bm, struct gengetopt_args_info *info)
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

static void
beatmap_set_difficulty(osux_beatmap *bm, struct gengetopt_args_info *info)
{
    bm->HPDrainRate  = 5;
    bm->CircleSize   = 5;
    bm->ApproachRate = 5;
    bm->OverallDifficulty = info->od_arg;
    if (info->abpm_arg > 0)
	bm->SliderMultiplier = 1.4 * (info->abpm_arg / info->bpm_arg);
    else
	bm->SliderMultiplier = 1.4;
    bm->SliderTickRate = 1;
}

static void
beatmap_set_tp(osux_beatmap *bm, struct gengetopt_args_info *info)
{
    osux_timingpoint *tp = g_malloc0(sizeof*tp);
    char *rep;
    rep = g_strdup_printf(
        "0,%g,4,%d,0,100,1,0", 60000. / info->bpm_arg, SAMPLE_TYPE_NORMAL);
    int err = osux_timingpoint_init(tp, rep, 14);
    g_free(rep);
    if (err) {
        osux_error("PROGRAMMING ERROR\n");
        abort();
    }

    bm->timingpoint_count   = 1;
    bm->timingpoint_bufsize = 1;
    bm->timingpoints = tp;
}

static void beatmap_init(osux_beatmap *bm, struct gengetopt_args_info *info)
{
    memset(bm, 0, sizeof*bm);
    beatmap_set_general(bm);
    beatmap_set_metadata(bm, info);
    beatmap_set_difficulty(bm, info);
    beatmap_set_tp(bm, info);
}

static void tg_set_ho(struct taiko_generator *tg, osux_hitobject *ho)
{
    while (tg_is_blank(tg))
	tg_next(tg);
    double offset = tg->offset + tg_rand(tg);
    make_taiko_circle(ho, offset, tg_get_sample(tg));
    tg_next(tg);
}

void
beatmap_add_ho(osux_beatmap *bm, struct gengetopt_args_info *info)
{
    struct taiko_generator tg;

    ALLOC_ARRAY(bm->hitobjects, bm->hitobject_bufsize, info->nb_ho_arg);
    bm->hitobject_count = info->nb_ho_arg;

    uint32_t seed = tg_seed(info);
    tg_init(&tg, info->pattern_arg, info->bpm_arg, info->random_arg, seed);
    for (int i = 0; i < info->nb_ho_arg; i++)
	tg_set_ho(&tg, &bm->hitobjects[i]);
    tg_free(&tg);
}

int main(int argc, char *argv[])
{
    int err = 0;
    struct gengetopt_args_info info;
    if (cmdline_parser(argc, argv, &info) != 0) {
        fprintf(stderr, "error parsing command line arguments\n");
        exit(EXIT_FAILURE);
    }

    if (info.bpm_arg < 5.) {
        fprintf(stderr, "BPM can't be this low (min=5.)\n");
        exit(EXIT_FAILURE);
    }
    if (info.od_arg < 0. || info.od_arg > 10.) {
	fprintf(stderr, "OD must be in [0, 10]\n");
	exit(EXIT_FAILURE);
    }
    if (info.nb_ho_arg < 0.) {
	fprintf(stderr, "Number of hitobjects must be positive\n");
	exit(EXIT_FAILURE);
    }
    if (info.random_arg < 0.)
	info.random_arg = -info.random_arg;


    osux_beatmap bm;
    beatmap_init(&bm, &info);
    beatmap_add_ho(&bm, &info);
    err = osux_beatmap_prepare(&bm);
    if (err) {
        fprintf(stderr, "Prepare beatmap has failed: %s\n",  osux_errmsg(err));
        goto end;
    }

    gchar *filename = osux_beatmap_default_filename(&bm);
    gchar *path = g_build_filename(info.output_dir_arg, filename, NULL);

    err = osux_beatmap_save(&bm, path);
    if (err) {
        fprintf(stderr, "Saving '%s' failed: %s\n", path, osux_errmsg(err));
    } else {
        if (!info.quiet_given)
            printf("Output file: '%s'\n", path);
    }

    g_free(filename);
    g_free(path);

end:
    osux_beatmap_free(&bm);
    cmdline_parser_free(&info);
    return EXIT_SUCCESS;
}
