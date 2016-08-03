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
#include <string.h>
#include <glib.h>

#include "osux/hitobject.h"
#include "osux/util.h"
#include "osux/error.h"
#include "osux/string2.h"

static int parse_slider(osux_hitobject *ho, char **split, int size)
{
    return 0;
}

static int parse_spinner(osux_hitobject *ho, char **split, int size)
{
    if (size < 6)
        return OSUX_ERR_INVALID_HITOBJECT;
        
    ho->spinner.end_offset = strtoull(split[5], NULL, 10);
    return 0;
}

static int parse_addon_hitsound(osux_hitobject *ho, char *addonstr)
{

    return -1;
}

static int parse_base_hitobject(osux_hitobject *ho, char **split, int size)
{
    g_assert(size >= 5);
    ho->x = atoi(split[0]);
    ho->y = atoi(split[1]);
    ho->offset = atoi(split[2]);
    ho->type = atoi(split[3]);
    ho->hitsound.sample = atoi(split[4]);

    if (string_contains(split[size-1], ':') &&
        !string_contains(split[size-1], '|'))
    {
        int err;
        if ((err = parse_addon_hitsound(ho, split[size-1])) < 0)
            return err;
    }
    
    return 0;
}

int osux_hitobject_init(osux_hitobject *ho, char *line, uint32_t osu_version)
{
    int err = OSUX_ERR_INVALID_HITOBJECT;
    char **split = g_strsplit(line, ",", 0);
    int size = strsplit_size(split);
    
    // memset(ho, 0, sizeof *ho);
    ho->_osu_version = osu_version;
    
    if (size < 5) {
        g_strfreev(split);
        return err;
    }
    
    if ((err = parse_base_hitobject(ho, split, size)) < 0)
        return err;
    
    int value = atoi(split[3]);
    int type = (value & 0x0F) & ~HITOBJECT_NEWCOMBO;

    switch (type) {
    case HITOBJECT_CIRCLE: err = 0; break;
    case HITOBJECT_SLIDER: err = parse_slider(ho, split, size); break;
    case HITOBJECT_SPINNER: err = parse_spinner(ho, split, size); break;
    default: err = OSUX_ERR_INVALID_HITOBJECT;   break;
    }
    g_strfreev(split);
    return err;
}

void osux_hitobject_print(osux_hitobject *ho, int version, FILE *f)
{
    fprintf(f, "%d,%d,%d,%d,%d",
	    ho->x, ho->y, ho->offset, ho->type, ho->hitsound.sample);
    
    switch ( HIT_OBJECT_TYPE(ho) ) {
    case HITOBJECT_SLIDER:
	fprintf(f, ",%c", ho->slider.type);
	for (unsigned i = 0; i < ho->slider.point_count; ++i)
	    fprintf(f, "|%d:%d", ho->slider.points[i].x, ho->slider.points[i].y);
	fprintf(f, ",%d,%.15g", ho->slider.repeat, ho->slider.length);
	if (ho->slider.edgehitsounds != NULL) {
	    fprintf(f, ",");
	    for (unsigned i = 0; i < ho->slider.repeat+1; ++i) {
		if (i >= 1) fprintf(f, "|");
		fprintf(f, "%d", ho->slider.edgehitsounds[i].sample);
	    }
	    fprintf(f, ",");
	    for (unsigned i = 0; i < ho->slider.repeat+1; ++i) {
		if (i >= 1) fprintf(f, "|");
		fprintf(f, "%d:%d", ho->slider.edgehitsounds[i].sample_type,
		       ho->slider.edgehitsounds[i].addon_sample_type);
	    }
	}
	break;
    case HITOBJECT_SPINNER:
	fprintf(f, ",%d", ho->spinner.end_offset);
	break;
    }
    if (ho->hitsound.have_addon) {
	fprintf(f, ",%d:%d:%d",
	       ho->hitsound.sample_type,
	       ho->hitsound.addon_sample_type,
	       ho->hitsound.sample_set_index);
	if (version > 11) {
	    fprintf(f, ":%d:%s",
		   ho->hitsound.volume,
		   ho->hitsound.sfx_filename);
	}
    }
    fprintf(f, "\r\n");
}

void osux_hitobject_free(osux_hitobject *ho)
{
    if ( HIT_OBJECT_IS_SLIDER(ho) ) {
	free(ho->slider.points);
        free(ho->slider.edgehitsounds);
    }
    free(ho->hitsound.sfx_filename);
}

