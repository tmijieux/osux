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
#include <glib.h>
#include <glib/gstdio.h>

#include "osux/beatmap.h"
#include "osux/error.h"
#include "osux/data.h"
#include "osux/parser.h"
#include "osux/hitobject.h"
#include "osux/timingpoint.h"
#include "osux/hitsound.h"
#include "osux/color.h"
#include "osux/locale.h"

#define PRINT_SECTION(section)                  \
    fprintf(f, "\r\n["#section"]\r\n")

// space afer colon
#define PRINT_STRING(map, file, Field)		\
    fprintf(f, #Field ": %s\r\n", m->Field)

#define PRINT_DOUBLE(map, file, Field)		\
    fprintf(f, #Field ": %.15g\r\n", m->Field)

#define PRINT_INT(map, file, Field)		\
    fprintf(f, #Field ": %ld\r\n", m->Field)

#define PRINT_STRING_V(map, file, field, value) \
    fprintf(f, #field ": %s\r\n", (value))

static int beatmap_print_internal(osux_beatmap const *m, FILE *f)
{
    fprintf(f, "osu file format v%d\r\n", m->osu_version);

    PRINT_SECTION( General );

    PRINT_STRING(m, f, AudioFilename);
    PRINT_INT(m, f, AudioLeadIn);
    PRINT_INT(m, f, PreviewTime);
    PRINT_INT(m, f, Countdown);
    PRINT_STRING_V(m, f, SampleSet, osux_sample_set_get_name(m->SampleSet));

    PRINT_DOUBLE(m, f, StackLeniency);
    PRINT_INT(m, f, Mode);
    PRINT_INT(m, f, LetterboxInBreaks);
    PRINT_INT(m, f, WidescreenStoryboard);

    PRINT_SECTION( Editor );
    if (m->bookmark_count) {
	fprintf(f, "Bookmarks: ");
	for (unsigned int i = 0; i < m->bookmark_count; ++i) {
	    if (i>=1)
		fprintf(f, ",");
	    fprintf(f, "%ld", m->bookmarks[i]);
	}
	fputs("\r\n", f);
    }

    PRINT_DOUBLE(m, f, DistanceSpacing);
    PRINT_INT(m, f, BeatDivisor);
    PRINT_INT(m, f, GridSize);
    PRINT_DOUBLE(m, f, TimelineZoom);

    #undef PRINT_STRING
    #undef PRINT_DOUBLE
    #undef PRINT_INT
    // no space afer colon
    #define PRINT_STRING(map, file, Field)      \
        fprintf(f, #Field ":%s\r\n", m->Field)	\

    #define PRINT_DOUBLE(map, file, Field)		\
        fprintf(f, #Field ":%.15g\r\n", m->Field)	\

    #define PRINT_INT(map, file, Field)		\
        fprintf(f, #Field ":%ld\r\n", m->Field)	\

    PRINT_SECTION( Metadata );
    PRINT_STRING(m, f, Title);
    if (m->TitleUnicode != NULL)
	PRINT_STRING(m, f, TitleUnicode);
    PRINT_STRING(m, f, Artist);
    if (m->ArtistUnicode != NULL)
	PRINT_STRING(m, f, ArtistUnicode);
    PRINT_STRING(m, f, Creator);
    PRINT_STRING(m, f, Version);
    PRINT_STRING(m, f, Source);
    PRINT_STRING(m, f, Tags);

    PRINT_INT(m, f, BeatmapID);
    PRINT_INT(m, f, BeatmapSetID);


    PRINT_SECTION( Difficulty );
    PRINT_DOUBLE(m, f, HPDrainRate);
    PRINT_DOUBLE(m, f, CircleSize);
    PRINT_DOUBLE(m, f, OverallDifficulty);
    PRINT_DOUBLE(m, f, ApproachRate);
    PRINT_DOUBLE(m, f, SliderMultiplier);
    PRINT_DOUBLE(m, f, SliderTickRate);

    PRINT_SECTION( Events );
    for (unsigned i = 0; i < m->event_count; ++i)
	osux_event_print(&m->events[i], f);

    PRINT_SECTION( TimingPoints );
    for (unsigned i = 0; i < m->timingpoint_count; ++i)
	osux_timingpoint_print(&m->timingpoints[i], f);

    PRINT_SECTION( Colours );
    for (unsigned i = 0; i < m->color_count; ++i)
        osux_color_print(f, &m->colors[i]);

    PRINT_SECTION( HitObjects );
    for (unsigned i = 0; i < m->hitobject_count; ++i)
	osux_hitobject_print(&m->hitobjects[i], m->osu_version, f);
    return 0;
}

int osux_beatmap_print(osux_beatmap const *m, FILE *f)
{
    SET_THREAD_LOCALE(cloc, "C");

    beatmap_print_internal(m, f);

    #ifdef _WIN32
    SET_THREAD_LOCALE(cloc, "");
    #elif __linux__
    RESTORE_THREAD_LOCALE(cloc);
    #endif

    return 0;
}

int osux_beatmap_save_full(
    osux_beatmap const *beatmap,
    char const *dirpath, char const *filename,
    bool use_default_filename)
{
    int err = 0;
    if (use_default_filename)
	filename = osux_beatmap_default_filename(beatmap);
    g_assert(filename != NULL);
    gchar *path = g_build_filename(dirpath, filename, NULL);
    err = osux_beatmap_save(beatmap, path);
    if (use_default_filename)
        g_free((gchar*) filename);
    g_free(path);
    return err;
}

int osux_beatmap_save(osux_beatmap const *beatmap, char const *path)
{
    int err = 0;
    FILE *file = g_fopen(path, "wb");

    if (file == NULL) {
        osux_error("%s: %s\n", path, strerror(errno));
        err = -OSUX_ERR_FILE_ERROR;
    } else {
        osux_beatmap_print(beatmap, file);
        fclose(file);
    }
    return err;
}

static char const sp_chr[] = "/";
static char const replace_chr[] = "_";

gchar *osux_beatmap_default_filename(const osux_beatmap *bm)
{
    gchar *name = g_strdup_printf(
        "%s - %s (%s) [%s].osu", bm->Artist, bm->Title, bm->Creator, bm->Version);

    unsigned len_name = strlen(name);
    unsigned len_spec = strlen(sp_chr);

    for (unsigned i = 0; i < len_name; i++)
	for (unsigned j = 0; j < len_spec; j++)
	    if (name[i] == sp_chr[j])
		name[i] = replace_chr[j];
    return name;
}
