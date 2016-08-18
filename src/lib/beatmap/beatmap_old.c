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
#include "osux/string2.h"
#include "osux/error.h"
#include "osux/data.h"
#include "osux/parser.h"
#include "osux/hitobject.h"
#include "osux/timingpoint.h"
#include "osux/color.h"


osux_beatmap DEFAULT_BEATMAP = { 0 };

#define PRINT_SECTION(section)                  \
    fprintf(f, "\r\n["#section"]\r\n")          \

#define PRINT_STRING(map, file, Field)		\
    fprintf(f, #Field ": %s\r\n", m->Field)	\

#define PRINT_DOUBLE(map, file, Field)		\
    fprintf(f, #Field ": %.15g\r\n", m->Field)	\

#define PRINT_INT(map, file, Field)		\
    fprintf(f, #Field ": %ld\r\n", m->Field)	\

int osux_beatmap_print(osux_beatmap const *m, FILE *f)
{
    fprintf(f, "osu file format v%d\r\n", m->osu_version);

    PRINT_SECTION( General );

    PRINT_STRING(m, f, AudioFilename);
    PRINT_INT(m, f, AudioLeadIn);
    PRINT_INT(m, f, PreviewTime);
    PRINT_INT(m, f, Countdown);
    PRINT_STRING(m, f, SampleSet);  // THIS IS SAMPLE TYPE! ;(

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

    fprintf(f, "Tags:");
    for (unsigned i = 0; i < m->tag_count; ++i) {
	fprintf(f, "%s", m->tags[i]);
	if (i < m->tag_count - 1)
	    fprintf(f, " ");
    }
    fputs("\r\n", f);

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

    PRINT_SECTION( TimingPoints );
    for (unsigned int i = 0; i < m->timingpoint_count; ++i)
	osux_timingpoint_print(&m->timingpoints[i], f);

    if (m->color_count) {
	PRINT_SECTION( Colours );
	for (unsigned int i = 0; i < m->color_count; ++i)
	    osux_color_print(f, &m->colors[i]);
	fputs("\r\n", f);
    }

    PRINT_SECTION( HitObjects );
    for (unsigned int i = 0; i < m->hitobject_count; ++i)
	osux_hitobject_print(&m->hitobjects[i], m->osu_version, f);
    return 0;
}

int osux_beatmap_save(
    osux_beatmap const *beatmap, char const *filename, bool use_default_filename)
{
    if (use_default_filename)
	filename = osux_beatmap_default_filename(beatmap);

    g_assert(filename != NULL);

    FILE *file = g_fopen(filename, "w+");
    if (file == NULL) {
        osux_error("%s: %s\n", filename, strerror(errno));
        return -1;
    }
    osux_beatmap_print(beatmap, file);
    fclose(file);

    if (use_default_filename)
        free((char*) filename);

    return 0;
}

#define BEATMAP_HITOBJECT_UPDATE_STAT(beatmap, hitobject)       \
    do {                                                        \
        if (HIT_OBJECT_IS_CIRCLE(hitobject))                    \
            ++ beatmap->circles;                                \
        else if (HIT_OBJECT_IS_SLIDER(hitobject))               \
            bm->sliders ++;                                     \
        else if (HIT_OBJECT_IS_SPINNER(hitobject))              \
            bm->spinners ++;                                    \
    } while (0)

static char const sp_chr[] = "/";
static char const replace_chr[] = "_";

char *osux_beatmap_default_filename(const osux_beatmap *bm)
{
    char *name = xasprintf(
        "%s - %s (%s) [%s].osu", bm->Artist, bm->Title, bm->Creator, bm->Version);

    unsigned len_name = strlen(name);
    unsigned len_spec = strlen(sp_chr);

    for (unsigned i = 0; i < len_name; i++)
	for (unsigned j = 0; j < len_spec; j++)
	    if (name[i] == sp_chr[j])
		name[i] = replace_chr[j];
    return name;
}
