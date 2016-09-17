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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "osux/compiler.h"
#include "osux/timingpoint.h"
#include "osux/color.h"

enum hitobject_type {

    HITOBJECT_CIRCLE   = 0x01,
    HITOBJECT_SLIDER   = 0x02, // drum roll (yellow) in taiko
    HITOBJECT_NEWCOMBO = 0x04,
    HITOBJECT_SPINNER  = 0x08, // shaker in taiko, bananas in ctb

    HITOBJECT_UNK1     = 0x10, // these unknown flags appear in a few map
    HITOBJECT_UNK2     = 0x20, // example: Nekomata Master - scar in the earth
    HITOBJECT_UNK3     = 0x40, // (Reisen Udongein) [Bunny Style].osu

    HITOBJECT_HOLD     = 0x80, // mania hold

    HITOBJECT_TYPE_MASK         = (HITOBJECT_CIRCLE | HITOBJECT_SLIDER |
                                   HITOBJECT_SPINNER | HITOBJECT_HOLD),
    HITOBJECT_UNKNOWN_FLAG_MASK = ~HITOBJECT_TYPE_MASK & ~HITOBJECT_NEWCOMBO,
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
#define HIT_OBJECT_TYPE(ho_ptr)    ((ho_ptr)->type & HITOBJECT_TYPE_MASK)
#define HIT_OBJECT_IS_NEWCOMBO(ho_ptr) ((ho_ptr)->type & HITOBJECT_NEWCOMBO)

#define HIT_OBJECT_IS_CIRCLE(x)  (HIT_OBJECT_TYPE(x) == HITOBJECT_CIRCLE)
#define HIT_OBJECT_IS_SLIDER(x)  (HIT_OBJECT_TYPE(x) == HITOBJECT_SLIDER)
#define HIT_OBJECT_IS_SPINNER(x) (HIT_OBJECT_TYPE(x) == HITOBJECT_SPINNER)
#define HIT_OBJECT_IS_HOLD(x) (HIT_OBJECT_TYPE(x) == HITOBJECT_HOLD)


typedef struct osux_point_ {
    int x;
    int y;
} osux_point;

typedef struct osux_hitsound_ {
    int sample;
    bool have_addon;
    int sample_type;
    int addon_sample_type;
    int sample_set_index;
    int volume;
    char *sfx_filename;
} osux_hitsound;

typedef struct osux_edgehitsound_ {
    // hitsound on slider extremities ('edge')
    int sample;
    int sample_type;
    int addon_sample_type;
} osux_edgehitsound;

typedef struct osux_slider_ {
    int type; // 'L', 'P' or 'B' (or 'C')
    uint32_t repeat;
    double length;

    uint32_t point_count;
    osux_point *points; // bézier control points
    osux_edgehitsound *edgehitsounds; // bufsize = 0 or 'repeat'
} osux_slider;

typedef struct osux_hitobject {
    int x;
    int y;
    int64_t offset;
    int64_t draw_offset;
    int64_t end_offset;
    uint32_t type;

    int combo_id;
    int combo_position;
    osux_color *combo_color;

    osux_slider slider;
    osux_hitsound hitsound;

    osux_timingpoint const *timingpoint;
    uint32_t _osu_version;

    char *details;
    char *errmsg;

    void *data;
} osux_hitobject;

int MUST_CHECK osux_hitobject_init(
    osux_hitobject *ho, char *line, uint32_t osu_version);
int osux_hitobject_prepare(osux_hitobject *ho,
                           int combo_id, int combo_pos, osux_color *color,
                           osux_timingpoint const *tp);
void osux_hitobject_print(osux_hitobject *ho, int version, FILE *f);
void osux_hitobject_free(osux_hitobject *ho);
void osux_hitobject_move(osux_hitobject *ho, osux_hitobject *target);
void osux_hitobject_copy(osux_hitobject *ho, osux_hitobject *target);
void osux_hitobject_apply_mods(osux_hitobject *ho, int mods);

#endif //OSUX_HIT_OBJECT_H
