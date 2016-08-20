#ifndef OSUX_TIMINGPOINT_H
#define OSUX_TIMINGPOINT_H
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

#include <stdbool.h>
#include <stdint.h>

/*
  timingpint format:

  - absolute timingpoint:
  offset,millisecond_per_beat,time_signature,sample_type,sample_set_index,
  volume,is_absolute=1,kiai

  -inherited:
  offset,slider_velocity_multiplier,time_signature,sample_type,
  sample_set_index,volume,is_absolute=0,kiai

*/

typedef struct osux_timingpoint {
    double offset;
    double millisecond_per_beat;
    double slider_velocity_multiplier;// percentage < 0
    double slider_velocity;

    int time_signature;
    int sample_type;
    int sample_set_index;
    int volume;

    bool inherited;
    bool kiai;

    struct osux_timingpoint const *last_non_inherited;
    int _osu_version;

    char *details;
    char *errmsg;
} osux_timingpoint;

#define TP_GET_BPM(timingpoint)                                 \
    (60.  * 1000. / (timingpoint)->millisecond_per_beat)

int osux_timingpoint_init(osux_timingpoint *tp,
                          char *line, uint32_t osu_version);

// this set both slider velocity and last_non_inherited timingpoint
int osux_timingpoint_prepare(
    osux_timingpoint *tp,
    osux_timingpoint const **last_non_inherited,
    double slider_velocity);

void osux_timingpoint_print(osux_timingpoint *tp, FILE *f);

// free timing point's internal resources
void osux_timingpoint_free(osux_timingpoint *tp);

#endif // OSUX_TIMINGPOINT_H
