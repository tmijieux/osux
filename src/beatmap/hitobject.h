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

#ifndef HIT_OBJECT_H
#define HIT_OBJECT_H

#include <stdint.h>

// HitObject '~~ TYPE'
#define HO_CIRCLE   1
#define HO_SLIDER   2
#define HO_NEWCOMBO 4 // <-- !!!
#define HO_SPINNER  8

// SLIDER type
#define SLIDER_LINE     'L'   // two points line
#define SLIDER_P        'P'   // three points line 
#define SLIDER_BEZIER   'B'   // 4 and more points line
/*
  Basically they all use the bezier interpolation
  but the formulas for the L and P types are simpler than
  the general case.
 */
#define TYPE_OF(ho_ptr)    ((ho_ptr)->type & (~HO_NEWCOMBO) & 0x0F)
// this get rid of the 'new_combo' flag to get the hit object's type
// more easily
struct point {
    int x;
    int y;
};

struct hitsound {
    int sample;
    
    // additionals:
    int additional;
    
    int st;
    int st_additional;
    int sample_set_index;
    int volume;
    char *sfx_filename;
};

struct add_hs {
    int sample;
    int st;
    int st_additional;
};

struct slider {
    int type;
    uint32_t repeat;
    double length;
    
    uint32_t point_count;
    struct point *pos;
    struct {
	int additional;
	struct add_hs *dat;
    } hs;
};

struct spinner {
    int end_offset;
};

struct hit_object {
    int x;
    int y;
    int offset;
    int type;
        
    struct slider sli;
    struct spinner spi;
    struct hitsound hs;
};

void ho_print(struct hit_object *ho, int version);
void ho_free(struct hit_object *ho);

#endif //HIT_OBJECT_H

