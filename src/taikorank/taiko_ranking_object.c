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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "print.h"
#include "bpm.h"
#include "taiko_ranking_object.h"

static char tro_char_type(const struct tr_object * o);

// percentage for equal
#define EPSILON 1.

#define TRO_SMALL_RADIUS 0.49        /* slightly smaller to avoid false superposition*/
#define TRO_BIG_RADIUS   0.707106781 /* sqrt(2.) / 2. */

struct tr_object * tro_copy(const struct tr_object * o, int nb)
{
    struct tr_object * copy = calloc(sizeof(struct tr_object), nb);
    memcpy(copy, o, sizeof(struct tr_object) * nb);
    return copy;
}

//--------------------------------------------------

double tro_get_radius(const struct tr_object * obj)
{
    if(tro_is_big(obj))
	return TRO_BIG_RADIUS;
    else
	return TRO_SMALL_RADIUS;
}

//--------------------------------------------------

int equal(double x, double y)
{
    return fabs(1. - x / y) <= (EPSILON / 100.);
}

//---------------------------------------------------
//---------------------------------------------------
//---------------------------------------------------

char * tro_str_state(const struct tr_object * o)
{
    switch(o->ps) {
    case GREAT:
	return "Great";
    case GOOD:
	return "Good";
    case MISS:
	return "Miss";
    case BONUS:
	return "Bonus";
    default:
	return "_";
    }
}

//---------------------------------------------------

static char tro_char_type(const struct tr_object * o)
{
    char c;
    if (tro_is_don(o))
	c = 'd';
    else if (tro_is_kat(o))
	c = 'k';
    else if (tro_is_roll(o))
	c = 'r';
    else
	c = 's';
    if (tro_is_big(o))
	return c + 'A' - 'a';
    else
	return c;
}

//---------------------------------------------------

void tro_print_yaml(const struct tr_object * o)
{
    fprintf(OUTPUT, "{");
    fprintf(OUTPUT, "offset: %d, ", o->offset);
    fprintf(OUTPUT, "type: %c, ", tro_char_type(o));
    fprintf(OUTPUT, "stars: {density_star: %g, ", o->density_star);
    fprintf(OUTPUT, "pattern_star: %g, ", o->pattern_star);
    fprintf(OUTPUT, "reading_star: %g, ", o->reading_star);
    fprintf(OUTPUT, "accuracy_star: %g, ", o->accuracy_star);
    fprintf(OUTPUT, "final_star: %g", o->final_star);
    fprintf(OUTPUT, "}}");
}

//---------------------------------------------------

void tro_print(const struct tr_object * obj, int filter)
{
    if((filter & FILTER_BASIC) != 0)
	fprintf(OUTPUT_INFO, "%d\t%d\t%c\t%.3g\t%s\t",
		obj->offset,
		obj->rest,
		tro_char_type(obj),
		obj->bpm_app,
		tro_str_state(obj));
    if((filter & FILTER_BASIC_PLUS) != 0)
	fprintf(OUTPUT_INFO, "%d\t%d\t%d\t%c\t%.3g\t%s\t",
		obj->offset,
		obj->end_offset,
		obj->rest,
		tro_char_type(obj),
		obj->bpm_app,
		tro_str_state(obj));
    if((filter & FILTER_ADDITIONNAL) != 0)
	fprintf(OUTPUT_INFO, "%d\t%d\t%g\t%g\t",
		tro_is_left_hand(obj),
		tro_is_right_hand(obj),
		obj->obj_app,
		obj->obj_dis);
    if((filter & FILTER_DENSITY) != 0)
	fprintf(OUTPUT_INFO, "%g\t%g\t%g\t",
		obj->density_raw,
		obj->density_color,
		obj->density_star);
    if((filter & FILTER_READING) != 0)
	fprintf(OUTPUT_INFO, "%d\t%d\t%.0f.\t%.3g\t",
		obj->offset_app,
		obj->offset_dis,
		obj->seen,
		obj->reading_star);
    if((filter & FILTER_READING_PLUS) != 0)
	fprintf(OUTPUT_INFO, "%d\t%d\t%d\t%d\t%d\t%.2g\t%.2g\t%.2g\t%g\t%.3g\t",
		obj->offset_app,
		obj->end_offset_app,
		obj->offset_dis,
		obj->end_offset_dis,
		obj->end_offset_dis_2,
		obj->line_a,
		obj->line_b,
		obj->line_b_end,
		obj->seen,
		obj->reading_star);
    if((filter & FILTER_ACCURACY) != 0)
	fprintf(OUTPUT_INFO, "%.3g\t%.3g\t%.3g\t%.3g\t",
		obj->slow,
		obj->hit_window,
		obj->spacing,
		obj->accuracy_star);
    if((filter & FILTER_PATTERN) != 0) {
	fprintf(OUTPUT_INFO, "%.3g\t%.3g\t%g\t",
		obj->proba,
		obj->pattern_freq,
		obj->pattern_star);
    }
    if((filter & FILTER_STAR) != 0)
	fprintf(OUTPUT_INFO, "%.3g\t%.3g\t%.3g\t%.3g\t%.3g\t",
		obj->density_star,
		obj->reading_star,
		obj->pattern_star,
		obj->accuracy_star,
		obj->final_star);

    fprintf(OUTPUT_INFO, "\n");
}
