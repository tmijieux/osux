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
#include <glib.h>

#include "osux/timingpoint.h"
#include "osux/util.h"
#include "osux/error.h"

int osux_timingpoint_init(osux_timingpoint *tp,
                          osux_timingpoint const **last_non_inherited,
                          char *line, uint32_t osu_version)
{
    char **split = g_strsplit(line, ",", 0);
    int size = strsplit_size(split);
    tp->_osu_version = osu_version;

    if (size < 7 || (osu_version > 5 && size < 8)) {
        g_strfreev(split);
        return OSUX_ERR_INVALID_HITOBJECT;
    }

    tp->offset = strtod(split[0], NULL);
    tp->millisecond_per_beat = strtod(split[1], NULL);
    tp->time_signature = atoi(split[2]);
    tp->sample_type = atoi(split[3]);
    tp->sample_set_index = atoi(split[4]);
    tp->volume = atoi(split[5]);
    tp->inherited = (atoi(split[6]) == 0);
    tp->kiai = (osu_version > 5) ? (atoi(split[7]) != 0) : false;

    if (tp->inherited) {
        tp->last_non_inherited = *last_non_inherited;
        g_assert(last_non_inherited != NULL);

        tp->slider_velocity_multiplier = tp->millisecond_per_beat;
        tp->millisecond_per_beat = tp->last_non_inherited->millisecond_per_beat;
    } else {
        *last_non_inherited = tp;
        tp->last_non_inherited = tp;
        tp->slider_velocity_multiplier = 0.;
    }
    g_strfreev(split);
    return 0;
}

void osux_timingpoint_print(osux_timingpoint *tp, FILE *f)
{
    fprintf(f, "%.15g,%.15g,%d,%d,%d,%d,%d,%d\n",
            tp->offset,
            (  tp->inherited ?
               tp->slider_velocity_multiplier :
               tp->millisecond_per_beat) ,
            tp->time_signature,
            tp->sample_type,
            tp->sample_set_index,
            tp->volume,
            tp->inherited ? 0 : 1,
            tp->kiai
    );
}

void osux_timingpoint_free(osux_timingpoint *tp)
{
    (void) tp; // nathing !
}
