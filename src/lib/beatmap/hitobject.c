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
#include <glib/gi18n.h>

#include "osux/hitobject.h"
#include "osux/hitsound.h"
#include "osux/util.h"
#include "osux/error.h"
#include "osux/string.h"
#include "osux/mods.h"

static int check_slider_type(char type)
{
    if (!(type == 'C' || type == 'L' || type == 'P' || type == 'B'))
        return -OSUX_ERR_INVALID_HITOBJECT_SLIDER_TYPE;
    return 0;
}

static int parse_slider_points(osux_hitobject *ho, char *pointstr)
{
    int err = 0;
    char **spl_points = g_strsplit(pointstr, "|", 0);
    unsigned size = strsplit_size(spl_points);

    ho->slider.point_count = size;
    ho->slider.points = g_malloc(size * sizeof*ho->slider.points);

    ho->slider.points[0].x = ho->x;
    ho->slider.points[0].y = ho->y;
    for (unsigned i = 1; i < size; ++i) {
        osux_point *pt = &ho->slider.points[i];
        if (sscanf(spl_points[i], "%d:%d", &pt->x, &pt->y) != 2) {
            err = -OSUX_ERR_INVALID_HITOBJECT_SLIDER_POINTS;
            g_free(ho->slider.points);
            ho->slider.points = NULL;
            break;
        }
    }

    g_strfreev(spl_points);
    return err;
}

static int parse_slider_sample_type(osux_hitobject *ho, char *ststr)
{
    int err = 0;
    char **split = g_strsplit(ststr, "|", 0);
    unsigned size = strsplit_size(split);
    if (size != ho->slider.repeat+1) {
        g_strfreev(split);
        g_free(ho->slider.edgehitsounds);
        ho->slider.edgehitsounds = NULL;
        return -OSUX_ERR_INVALID_HITOBJECT_EDGE_SAMPLE_TYPE;
    }

    if (ho->slider.edgehitsounds == NULL) {
        ho->slider.edgehitsounds = g_malloc0(
            size * sizeof*ho->slider.edgehitsounds);
    }
    for (unsigned i = 0; i < size; ++i) {
        osux_edgehitsound *eht = &ho->slider.edgehitsounds[i];
        if (sscanf(split[i], "%d:%d", &eht->sample_type,
                   &eht->addon_sample_type) != 2) {
            err = -OSUX_ERR_INVALID_HITOBJECT_EDGE_SAMPLE_TYPE;
            break;
        }
    }
    g_strfreev(split);
    return err;
}

static int parse_slider_sample(osux_hitobject *ho, char *samplestr)
{
    char **split = g_strsplit(samplestr, "|", 0);
    unsigned size = strsplit_size(split);
    if (size != ho->slider.repeat+1) {
        g_strfreev(split);
        g_free(ho->slider.edgehitsounds);
        ho->slider.edgehitsounds = NULL;
        return -OSUX_ERR_INVALID_HITOBJECT_EDGE_SAMPLE;
    }

    if (ho->slider.edgehitsounds == NULL) {
        ho->slider.edgehitsounds = g_malloc0(
            size * sizeof*ho->slider.edgehitsounds);
    }
    for (unsigned i = 0; i < size; ++i) {
        osux_edgehitsound *eht = &ho->slider.edgehitsounds[i];
        eht->sample = atoi(split[i]);
    }
    g_strfreev(split);
    return 0;
}

static inline void compute_slider_end_offset(
    osux_hitobject *ho, osux_timingpoint const *tp)
{
    double length = ho->slider.length * ho->slider.repeat;
    length *= tp->millisecond_per_beat / (100. * tp->slider_velocity);
    ho->end_offset = ho->offset + length;
}

int osux_hitobject_prepare(osux_hitobject *ho,
                           int combo_id, int combo_pos, osux_color *color,
                           osux_timingpoint const *tp)
{
    if (HIT_OBJECT_IS_SLIDER(ho))
        compute_slider_end_offset(ho, tp);

    ho->timingpoint = tp;
    ho->details = g_strdup_printf(
        "%s%s%s",
        ho->hitsound.sample & SAMPLE_WHISTLE?_("Whistle "):"",
        ho->hitsound.sample & SAMPLE_FINISH?_("Finish "):"",
        ho->hitsound.sample & SAMPLE_CLAP ?_("Clap "):"");
    ho->combo_color = color;
    ho->combo_id = combo_id;
    ho->combo_position = combo_pos;
    return 0;
}

static int parse_slider(osux_hitobject *ho, char **split, unsigned size)
{
    int err = 0;
    char *curve = split[5];

    ho->slider.type = curve[0];
    if ((err = check_slider_type(ho->slider.type)) < 0)
        return err;

    if ((err = parse_slider_points(ho, curve)) < 0)
        return err;

    ho->slider.repeat = atoi(split[6]);
    ho->slider.length = g_ascii_strtod(split[7], NULL);
    ho->end_offset = 0; // set later

    for (unsigned i = 8; i < size; ++i) {
        if (string_contains(split[i], '|') && string_contains(split[i], ':')) {
            // edge hitsound sample type
            if ((err = parse_slider_sample_type(ho, split[i])) < 0)
                return err;
        } else if (string_contains(split[i], '|') &&
                   !string_contains(split[i], ':')) {
            // edge hitsound sample
            if ((err = parse_slider_sample(ho, split[i])) < 0)
                return err;
        }
    }
    return 0;
}

static int parse_spinner(osux_hitobject *ho, char **split, unsigned size)
{
    if (size < 6)
        return -OSUX_ERR_INVALID_HITOBJECT_SPINNER;

    ho->end_offset = g_ascii_strtoull(split[5], NULL, 10);
    return 0;
}

static int parse_hold(osux_hitobject *ho, char **split, unsigned size)
{
    if (size < 6)
        return -OSUX_ERR_INVALID_HITOBJECT_HOLD;

    ho->end_offset = strtoull(split[5], NULL, 10);
    return 0;
}

static int parse_addon_hitsound(osux_hitobject *ho, char *addonstr)
{
    char **split = g_strsplit(addonstr, ":", 0);
    unsigned size = strsplit_size(split);

    if (HIT_OBJECT_IS_HOLD(ho)) {
        ++ split;
        -- size;
    }

    if (size < 2 || (ho->_osu_version > 10 && size < 3)
        || (ho->_osu_version > 11 && size < 4)) {
        g_strfreev(split);
        return -OSUX_ERR_INVALID_HITOBJECT_ADDON_HITSOUND;
    }

    ho->hitsound.sample_type = atoi(split[0]);
    ho->hitsound.addon_sample_type = atoi(split[1]);

    if (split[2] != NULL) {
        ho->hitsound.sample_set_index = atoi(split[2]);
        if (split[3] != NULL) {
            ho->hitsound.volume = atoi(split[3]);
            if (split[4] != NULL)
                ho->hitsound.sfx_filename = g_strdup(split[4]);
            else
                ho->hitsound.sfx_filename = g_strdup("");
        } else {
            ho->hitsound.volume = 70;
            ho->hitsound.sfx_filename = g_strdup("");
        }
    } else {
        ho->hitsound.sample_set_index = 0;
        ho->hitsound.volume = 70;
        ho->hitsound.sfx_filename = g_strdup("");
    }

    if (HIT_OBJECT_IS_HOLD(ho))
        -- split;

    g_strfreev(split);
    ho->hitsound.have_addon = true;
    return 0;
}

static int parse_base_hitobject(osux_hitobject *ho, char **split, unsigned size)
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
    int err;
    char **split = g_strsplit(line, ",", 0);
    unsigned size = strsplit_size(split);
    memset(ho, 0, sizeof *ho);

    ho->_osu_version = osu_version;

    if (size < 5) {
        g_strfreev(split);
        return -OSUX_ERR_INVALID_HITOBJECT;
    }

    if ((err = parse_base_hitobject(ho, split, size)) < 0)
        return err;

    int value = atoi(split[3]);
    /*
      if (value & HITOBJECT_UNKNOWN_FLAG_MASK) {
      osux_warning("hitobject use extra flag with unknown purposes: %d\n",
      value & HITOBJECT_UNKNOWN_FLAG_MASK);
      }
    */

    int type = value & HITOBJECT_TYPE_MASK;

    switch (type) {
    case HITOBJECT_CIRCLE: err = 0; ho->end_offset = ho->offset; break;
    case HITOBJECT_SLIDER: err = parse_slider(ho, split, size); break;
    case HITOBJECT_SPINNER: err = parse_spinner(ho, split, size); break;
    case HITOBJECT_HOLD: err = parse_hold(ho, split, size); break;
    default: err = -OSUX_ERR_INVALID_HITOBJECT_TYPE; break;
    }
    g_strfreev(split);
    return err;
}

static void osux_slider_print(osux_hitobject *ho, int version, FILE *f)
{
    (void) version;
    fprintf(f, ",%c", ho->slider.type);
    for (unsigned i = 1; i < ho->slider.point_count; ++i)
        fprintf(f, "|%d:%d", ho->slider.points[i].x, ho->slider.points[i].y);
    fprintf(f, ",%d,%.15g", ho->slider.repeat, ho->slider.length);
    if (ho->slider.edgehitsounds != NULL) {
        fprintf(f, ",");
        // print edge hit sounds sample:
        for (unsigned i = 0; i < ho->slider.repeat+1; ++i) {
            if (i >= 1) fprintf(f, "|");
            fprintf(f, "%d", ho->slider.edgehitsounds[i].sample);
        }
        fprintf(f, ",");
        // print edge hit sounds sample types (normal|addon):
        for (unsigned i = 0; i < ho->slider.repeat+1; ++i) {
            if (i >= 1) fprintf(f, "|");
            fprintf(f, "%d:%d", ho->slider.edgehitsounds[i].sample_type,
                    ho->slider.edgehitsounds[i].addon_sample_type);
        }
    }
}

void osux_hitobject_print(osux_hitobject *ho, int version, FILE *f)
{
    fprintf(f, "%d,%d,%ld,%d,%d",
	    ho->x, ho->y, ho->offset, ho->type, ho->hitsound.sample);
    switch ( HIT_OBJECT_TYPE(ho) ) {
    case HITOBJECT_SLIDER:
        osux_slider_print(ho, version, f);
	break;
    case HITOBJECT_SPINNER:
    case HITOBJECT_HOLD:
	fprintf(f, ",%ld", ho->end_offset);
	break;
    default:
        break;
    }
    if (ho->hitsound.have_addon) {
	fprintf(f, "%c%d:%d:%d",
                HIT_OBJECT_IS_HOLD(ho) ? ':' : ',',
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
	g_free(ho->slider.points);
        g_free(ho->slider.edgehitsounds);
    }
    g_free(ho->hitsound.sfx_filename);
    g_free(ho->details);
    g_free(ho->errmsg);
    if (ho->free_data != NULL)
        ho->free_data(ho->data);
    memset(ho, 0, sizeof*ho);
}

void osux_hitobject_move(osux_hitobject *ho, osux_hitobject *target)
{
    *target = *ho;
    memset(ho, 0, sizeof *ho);
}

void osux_hitobject_copy(osux_hitobject *ho, osux_hitobject *cpy)
{
    *cpy = *ho;
    cpy->hitsound.sfx_filename = g_strdup(ho->hitsound.sfx_filename);
    cpy->details = g_strdup(ho->details);
    cpy->errmsg = g_strdup(ho->errmsg);

    if (HIT_OBJECT_IS_SLIDER(ho)) {
        cpy->slider.points = ARRAY_DUP(ho->slider.points, ho->slider.point_count);
        cpy->slider.edgehitsounds = ARRAY_DUP(
            ho->slider.edgehitsounds, ho->slider.repeat+1);
    }

    // cpy->timingpoint = NULL;
    // this field has no meaning outside of beatmaps
}

void osux_hitobject_apply_mods(osux_hitobject *ho, int mods)
{
    if (mods & MOD_HARDROCK) {
        ho->y = 384 - ho->y;
        if (HIT_OBJECT_IS_SLIDER(ho)) {
            for (unsigned i = 0; i < ho->slider.point_count; ++i) {
                int y = ho->slider.points[i].y;
                ho->slider.points[i].y = 384 - y;
            }
        }
    }
}

void osux_hitobject_set_data(osux_hitobject *ho,
                             gpointer data, GDestroyNotify free_data)
{
    if (ho->free_data != NULL)
        ho->free_data(ho->data);
    ho->data = data;
    ho->free_data = free_data;
}
