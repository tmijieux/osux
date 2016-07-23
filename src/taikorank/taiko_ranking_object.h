/*
 *  Copyright (©) 2015-2016 Lucas Maugère, Thomas Mijieux
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

#ifndef TRO_H
#define TRO_H

#include <gts.h>
#include "util/table.h"

enum played_state {
    GREAT, GOOD, MISS, BONUS
};

#define TRO_D   (1 << 0) // don
#define TRO_K   (1 << 1) // kat
#define TRO_S   (1 << 2) // spinner
#define TRO_R   (1 << 3) // roll
#define TRO_BIG (1 << 4) // big note
#define TRO_LH  (1 << 5) // left hand
#define TRO_RH  (1 << 6) // right hand

#define TRO_HAND (TRO_LH | TRO_RH) // hand filter
#define TRO_DK   (TRO_K  | TRO_D ) // dk filter

struct tr_object
{
    // ---- basic data
    int offset;
    int end_offset;        // printed in basic+
    int bf;                // bitfield
    double bpm_app;

    enum played_state ps;

    const struct tr_object * objs;

    // ---- additionnal data
    int length;
    int rest;            // printed in basic

    // ---- pattern data
    double proba;
    double pattern_freq;
    char type;
    struct table * patterns;
  
    // ---- density data
    double density_raw;
    double density_color;

    // ---- reading data
    double obj_app;      // nb obj displayable on screen, without overlapping
    double obj_dis;      // nb obj where disappear
    int offset_app;      // in reading,  border left appear
    int end_offset_app;  // in reading+, border right appear
    int offset_dis;      // in reading,  border right disappear
    int end_offset_dis;  // in reading+, border left disappear
    int end_offset_dis_2; // end_offset_dis if the object was always visible

    struct table * obj_h;

    double line_a;
    double line_b;       // nb obj before app
    double line_b_end;   // nb obj before end_app

    GtsSurface * mesh;
    int count;
    int done;     // used as a bitfield

    double seen;

    // ---- accuracy star
    double slow;         // slow, hard to acc
    double hit_window;
    double spacing;

    // ---- star
    double density_star;
    double reading_star;
    double pattern_star;
    double accuracy_star;
    double final_star;
};

//-------------------------------------------------------------

struct tr_object * tro_copy(const struct tr_object * o, int nb);

void tro_print_yaml(const struct tr_object * o);
void tro_print(const struct tr_object * obj, int filter);

int equal(double x, double y);

int tro_get_length(const struct tr_object * obj);
double tro_get_radius(const struct tr_object * obj);

//-------------------------------------------------------------
// inline 
static inline int tro_is_big(const struct tr_object * obj) {
    return obj->bf & TRO_BIG;
}

static inline int tro_is_bonus(const struct tr_object * obj) {
    return obj->bf & (TRO_R | TRO_S);
}

static inline int tro_is_roll(const struct tr_object * obj) {
    return obj->bf & TRO_R;
}

static inline int tro_is_spinner(const struct tr_object * obj) {
    return obj->bf & TRO_S;
}

static inline int tro_is_circle(const struct tr_object * obj) {
    return obj->bf & (TRO_D | TRO_K);
}

static inline int tro_is_kat(const struct tr_object * obj) {
    return obj->bf & TRO_K;
}

static inline int tro_is_don(const struct tr_object * obj) {
    return obj->bf & TRO_D;
}

static inline int tro_is_right_hand(const struct tr_object * obj) {
    return !!(obj->bf & TRO_RH);
}

static inline int tro_is_left_hand(const struct tr_object * obj) {
    return !!(obj->bf & TRO_LH);
}

static inline int tro_are_same_hand(const struct tr_object * o1, 
				    const struct tr_object * o2)
{
    return TRO_HAND & o1->bf & o2->bf;
}

static inline int tro_are_same_type(const struct tr_object * o1, 
				    const struct tr_object * o2)
{
    if(TRO_S & (o1->bf | o2->bf)) // one is spinner
	return 1; // d and k are played
    if(TRO_R & (o1->bf | o2->bf)) // one is roll
	return 0; // suppose you play the easier...
    return (TRO_DK & o1->bf & o2->bf);
}

#endif
