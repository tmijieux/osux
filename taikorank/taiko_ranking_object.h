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

#define MSEC_IN_MINUTE 60000.

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
    int end_offset;     // printed in basic+
    int bf; // bitfield
    double bpm_app;
  
    enum played_state ps;

    // ---- additionnal data
    int length;
    int rest;            // printed in basic
    double obj_app;      // nb obj displayable on screen, without overlapping
    double obj_dis;      // nb obj where disappear
    int offset_app;      // printed in reading,  border left appear
    int end_offset_app;  // printed in reading+, border right appear
    int offset_dis;      // printed in reading,  border right disappear
    int end_offset_dis;  // printed in reading+, border left disappear
    double c_app;     // bpm_app * end_offset_app
    double c_end_app; // bpm_app * offset_app

    // ---- pattern data
    double proba;
    double pattern;
    struct pattern **patterns;
  
    // ---- density data
    double density_raw;
    double density_color;

    // ---- reading data
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

void tro_print_yaml(struct tr_object * o);
void tro_print(struct tr_object * obj, int filter);

double mpb_to_bpm (double mpb);
int equal (double x, double y);

int tro_get_length(struct tr_object * obj);
double tro_get_size(struct tr_object * obj);

int tro_are_same_hand(struct tr_object * o1, struct tr_object * o2);
int tro_are_same_type(struct tr_object * o1, struct tr_object * o2);
int tro_are_same_density(struct tr_object *o1, struct tr_object *o2);

//-------------------------------------------------------------
// tro_table

struct tro_table {
    struct tr_object ** t;
    int l;    // current len
    int size; // max len
};

struct tro_table * tro_table_new(int l);
struct tro_table * tro_table_from_vl(int l, ...);

void tro_table_add(struct tro_table * t, struct tr_object * obj);
struct tro_table * tro_table_from_array(struct tr_object ** t,int l);
void tro_table_free(struct tro_table * t);

//-------------------------------------------------------------
// inline 
inline int tro_is_big(struct tr_object * obj) {
    return obj->bf & TRO_BIG;
}

inline int tro_is_bonus(struct tr_object * obj) {
    return obj->bf & (TRO_R | TRO_S);
}

inline int tro_is_roll(struct tr_object * obj) {
    return obj->bf & TRO_R;
}

inline int tro_is_spinner(struct tr_object * obj) {
    return obj->bf & TRO_S;
}

inline int tro_is_circle(struct tr_object * obj) {
    return obj->bf & (TRO_D | TRO_K);
}

inline int tro_is_kat(struct tr_object * obj) {
    return obj->bf & TRO_K;
}

inline int tro_is_don(struct tr_object * obj) {
    return obj->bf & TRO_D;
}

inline int tro_is_right_hand(struct tr_object * obj) {
    return !!(obj->bf & TRO_RH);
}

inline int tro_is_left_hand(struct tr_object * obj) {
    return !!(obj->bf & TRO_LH);
}

#endif
