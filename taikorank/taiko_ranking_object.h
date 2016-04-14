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

struct tr_object
{
    // ---- basic data
    int offset;
    int end_offset;     // printed in basic+
    char type;          // d D k K s(spinner) r(roll) R(roll)
    double bpm_app;
  
    enum played_state ps;

    // ---- additionnal data
    int l_hand;          
    int r_hand;          
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

int tro_is_big (struct tr_object * obj);
int tro_is_bonus (struct tr_object * obj);
int tro_is_slider (struct tr_object * obj);
int tro_is_circle (struct tr_object * obj);
int tro_is_kat (struct tr_object * obj);
int tro_is_don (struct tr_object * obj);

int tro_are_same_hand (struct tr_object * obj1,
		       struct tr_object * obj2);
int tro_are_same_type (struct tr_object * obj1,
		       struct tr_object * obj2);
int tro_are_same_density (struct tr_object * obj1,
			  struct tr_object * obj2);

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

#endif
