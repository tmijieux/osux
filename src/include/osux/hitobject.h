#ifndef OSUX_HIT_OBJECT_H
#define OSUX_HIT_OBJECT_H

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

#include <stdint.h>
#include <stdbool.h>

enum hitobject_type {
    HITOBJECT_CIRCLE   = 1,
    HITOBJECT_SLIDER   = 2,
    HITOBJECT_NEWCOMBO = 4,
    HITOBJECT_SPINNER  = 8,
};

enum slider_type {
    SLIDER_LINE     = 'L',   // two points line
    SLIDER_P        = 'P',   // three points line 
    SLIDER_BEZIER   = 'B',   // 4 and more points line
    SLIDER_C        = 'C', // seen on v5, unknown usage
};

/*
  Basically they all use the bezier interpolation
  but the formulas for the L and P types are simpler than
  the general case.
 */

// get rid of the 'new_combo' flag to get the hit object's type more easily
#define HIT_OBJECT_TYPE(ho_ptr)    ((ho_ptr)->type & (~HITOBJECT_NEWCOMBO) & 0x0F)

#define HIT_OBJECT_IS_CIRCLE(x)  (HIT_OBJECT_TYPE(x) == HITOBJECT_CIRCLE)
#define HIT_OBJECT_IS_SLIDER(x)  (HIT_OBJECT_TYPE(x) == HITOBJECT_SLIDER)
#define HIT_OBJECT_IS_SPINNER(x) (HIT_OBJECT_TYPE(x) == HITOBJECT_SPINNER)


typedef struct osux_point {
    int x;
    int y;
} osux_point;

typedef struct osux_hitsound {
    int sample;
    bool have_addon;    
    int sample_type;
    int addon_sample_type;
    int sample_set_index;
    int volume;
    char *sfx_filename;
} osux_hitsound;


typedef struct osux_edgehitsound {
    // hitsound on slider extremities ('edge')
    int sample;
    int sample_type;
    int addon_sample_type;
} osux_edgehitsound;

typedef struct osux_slider {
    int type; // 'L', 'P' or 'B' (or 'C')
    uint32_t repeat;
    double length;
    
    uint32_t point_count;
    osux_point *points;
    osux_edgehitsound *edgehitsounds; // bufsize = 0 or 'repeat'
} osux_slider;

typedef struct osux_spinner {
    unsigned end_offset;
} osux_spinner;

typedef struct osux_hitobject {
    int x;
    int y;
    uint32_t offset;
    uint32_t type;
        
    osux_slider slider;
    osux_spinner spinner;
    osux_hitsound hitsound;

    uint32_t _osu_version;
} osux_hitobject;

int osux_hitobject_init(osux_hitobject *ho, char *line, uint32_t osu_version);
void osux_hitobject_print(osux_hitobject *ho, int version, FILE *f);
void osux_hitobject_free(osux_hitobject *ho);

#endif //OSUX_HIT_OBJECT_H

