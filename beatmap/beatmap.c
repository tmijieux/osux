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

#include "util/error.h"
#include "util/data.h"
#include "beatmap.h"
#include "parser/parser.h"
#include "hitobject.h"
#include "timingpoint.h"

#include "storyboard.h"
#include "color.h"

osux_beatmap DEFAULT_MAP = { 0 };


#define PRINT_SECTION(section)		\
    fprintf(f, "\r\n["#section"]\r\n")	\

#define PRINT_STRING(map, file, Field)		\
    fprintf(f, #Field ": %s\r\n", m->Field)	\
    
#define PRINT_DOUBLE(map, file, Field)		\
    fprintf(f, #Field ": %.15g\r\n", m->Field)	\

#define PRINT_INT(map, file, Field)		\
    fprintf(f, #Field ": %d\r\n", m->Field)	\

int osux_beatmap_print(const osux_beatmap *m, FILE *f)
{
    if (m->bom) {
	fprintf(f, "%c%c%c", 0xef, 0xbb, 0xbf);
    }
    fprintf(f, "osu file format v%d\r\n", m->version);
    
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
    if (m->bkmkc) {
	fprintf(f, "Bookmarks: ");
	for (unsigned int i = 0; i < m->bkmkc; ++i){
	    if (i>=1) printf(",");
	    fprintf(f, "%d", m->Bookmarks[i]);
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

#define PRINT_STRING(map, file, Field)		\
    fprintf(f, #Field ":%s\r\n", m->Field)	\
    
#define PRINT_DOUBLE(map, file, Field)		\
    fprintf(f, #Field ":%.15g\r\n", m->Field)	\

#define PRINT_INT(map, file, Field)		\
    fprintf(f, #Field ":%d\r\n", m->Field)	\

    
    PRINT_SECTION( Metadata );
    PRINT_STRING(m, f, Title);
    PRINT_STRING(m, f, TitleUnicode);
    PRINT_STRING(m, f, Artist);
    PRINT_STRING(m, f, ArtistUnicode);
    PRINT_STRING(m, f, Creator);
    PRINT_STRING(m, f, Version);
    PRINT_STRING(m, f, Source);
    printf("Tags:");
    for (unsigned int i = 0; i < m->tagc; ++i) {
	fprintf(f, "%s", m->Tags[i]);
	if (i < m->tagc-1) fprintf(f, " ");
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
    for (unsigned int i = 0; i < m->tpc; ++i)
	tp_print(&m->TimingPoints[i]);

    if (m->colc) {
	PRINT_SECTION( Colours );
	for (unsigned int i = 0; i < m->colc; ++i)
	    col_print(f, &m->Colours[i], i+1);
	fputs("\r\n", f);
    }
    
    PRINT_SECTION( HitObjects );
    for (unsigned int i = 0; i < m->hoc; ++i)
	ho_print(&m->HitObjects[i], m->version);
    return 0;
}

int osux_beatmap_save(const char *filename, const osux_beatmap* bm)
{
    FILE *f = fopen(filename, "w+");
    if (NULL == f) {
        osux_error("%s: %s\n", filename, strerror(errno));
        return -1;
    }
    osux_beatmap_print(bm, f);
    fclose(f);
    return 0;
}

int osux_beatmap_prepare(osux_beatmap *bm, const char *filename)
{
    bm->circles = bm->sliders = bm->spinners = 0;
    
    for (unsigned i = 0; i < bm->hoc; ++i) {
        if (HO_IS_CIRCLE(bm->HitObjects[i]))
            ++ bm->circles ;
        else if (HO_IS_SLIDER(bm->HitObjects[i]))
            bm->sliders ++;
        else if (HO_IS_SPINNER(bm->HitObjects[i]))
            bm->spinners ++;
    }
    int l = strlen(filename);
    for (int i = l; i >= 0; --i) {
        if ('/' == filename[i]) {
            bm->osu_filename = strdup(&filename[i+1]);
            bm->path = strndup(filename, i-1);
            break;
        }
    }
    return 0;
}

int osux_beatmap_open(const char *filename, osux_beatmap **beatmap)
{
	osux_beatmap *(*osux_parse_beatmap)(const char *filename);
	osux_parse_beatmap = osux_get_parser();
	if (osux_parse_beatmap == NULL)
		return -1;
	
    *beatmap = osux_parse_beatmap(filename);
    if (NULL == *beatmap)
        return -1;
    osux_beatmap_prepare(*beatmap, filename);
    return 0;
}

int osux_beatmap_reopen(osux_beatmap *bm_in, osux_beatmap **bm_out)
{
    int ret;
    char *path;
    if (NULL == bm_in) {
        *bm_out = NULL;
        return -1;
    }
    path = osux_prefix_path(osux_get_song_path(), bm_in->osu_filename);
    ret = osux_beatmap_open(path, bm_out);
    free(path);
    return ret;
}

static void osux_beatmap_free(osux_beatmap *m)
{
    free(m->AudioFilename);
    free(m->SampleSet);
    free(m->Bookmarks);
    free(m->Title);
    free(m->TitleUnicode);
    free(m->Artist);
    free(m->ArtistUnicode);
    free(m->Creator);
    free(m->Version);
    free(m->Source);

    for (unsigned int i = 0; i < m->tagc; ++i)
	free(m->Tags[i]);
    free(m->Tags);

    for (unsigned int i = 0; i < m->tpc; ++i)
	tp_free(&m->TimingPoints[i]);
    free(m->TimingPoints);
    free(m->Colours);

    for (unsigned int i = 0; i < m->hoc; ++i)
	ho_free(&m->HitObjects[i]);

    free(m->HitObjects);
    free(m);
}

int osux_beatmap_close(osux_beatmap *beatmap)
{
    if (NULL != beatmap) {
        osux_beatmap_free(beatmap);
        return 0;
    }
    return -1;
}

