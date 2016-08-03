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
#include "osux/timingpoint.h"

int osux_timingpoint_init(osux_timingpoint *tp,
                          osux_timingpoint const **last_non_inherited,
                          char *line, uint32_t osu_version)
{
    
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
