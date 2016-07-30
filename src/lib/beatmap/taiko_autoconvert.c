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
#include <stdlib.h>
#include <math.h>

#include "osux/beatmap.h"
#include "osux/hitobject.h"
#include "osux/timingpoint.h"
#include "osux/taiko_autoconvert.h"
#include "osux/list.h"

#define OFFSET_EQUAL_PERCENTAGE 0.05

struct taiko_converter {
    struct hit_object *ho;
    double mpb; // ms per beat
    double mpt; // ms per tick
    double length;
};

static inline int offset_eq(double o1, double o2);
static inline int offset_le(double o1, double o2);

static double ho_slider_length(const struct hit_object *ho,
			       const struct timing_point *tp,
			       double sv_multiplier);
static struct hit_object * ho_taiko_new(
    double offset, const struct hit_object *ho_sli_src, int add_index);
static void ho_taiko_free(struct hit_object * ho);

static struct taiko_converter * taiko_converter_new(
    struct hit_object *ho, const struct timing_point *tp,
    double sv, double tick_rate);
static void taiko_converter_free(struct taiko_converter * tc);

static void taiko_converter_slider_to_circles_normal(
    const struct taiko_converter *tc, struct osux_list *ho_list);
static void taiko_converter_slider_to_circles_repeat(
    const struct taiko_converter *tc, struct osux_list *ho_list);
static void taiko_converter_convert_slider(
    const struct taiko_converter *tc, struct osux_list *ho_list);
static void taiko_converter_convert(
    const struct taiko_converter *tc, struct osux_list *ho_list);

static struct osux_list * taiko_autoconvert_ho_list(const osux_beatmap *bm);
static struct hit_object * osux_list_to_ho_array(const struct osux_list *l);

//---------------------------------------------------------------

static inline int offset_eq(double o1, double o2)
{
    return fabs(1. - o1 / o2) <= OFFSET_EQUAL_PERCENTAGE;
}

static inline int offset_le(double o1, double o2)
{
    return (o1 <= o2) || offset_eq(o1, o2);
}

//---------------------------------------------------------------

static double ho_slider_length(const struct hit_object *ho,
			       const struct timing_point *tp,
			       double sv_multiplier)
{
    double sv = sv_multiplier;
    if (!tp->uninherited)
	sv *= -100. / tp->svm;
    return ((ho->sli.length * ho->sli.repeat * tp->last_uninherited->mpb) /
	    (100 * sv));
}

//---------------------------------------------------------------

static struct hit_object * ho_taiko_new(
    double offset, const struct hit_object *ho_sli_src, int add_index)
{
    struct hit_object * ho = calloc(sizeof(*ho), 1);
    ho->spi.end_offset = -1; // flag the object
    ho->offset = offset;
    ho->type   = HO_CIRCLE;
    ho->hs.sample = ho_sli_src->hs.sample;
    if (ho_sli_src->sli.hs.additional)
	ho->hs.sample = ho_sli_src->sli.hs.dat[add_index].sample;
    return ho;
}

static void ho_taiko_free(struct hit_object * ho)
{
    if (HO_IS_CIRCLE(*ho) && ho->spi.end_offset == -1)
	free(ho);
}

//---------------------------------------------------------------

static struct taiko_converter * taiko_converter_new(
    struct hit_object *ho, const struct timing_point *tp,
    double sv, double tick_rate)
{
    struct taiko_converter * tc = malloc(sizeof(*tc));
    tc->ho = ho;
    tc->mpb = tp->last_uninherited->mpb;
    tc->mpt = tc->mpb / tick_rate;
    if (HO_IS_SLIDER(*ho))
	tc->length = ho_slider_length(ho, tp, sv);
    else
	tc->length = 0;
    return tc;
}

static void taiko_converter_free(struct taiko_converter * tc)
{
    free(tc);
}

void taiko_converter_print(const struct taiko_converter * tc)
{
    fprintf(stderr, "Taiko_converter:\n");
    fprintf(stderr, "\toffset: %d\n", tc->ho->offset);
    fprintf(stderr, "\tmpb: %g\n", tc->mpb);
    fprintf(stderr, "\tmpt: %g\n", tc->mpt);
    fprintf(stderr, "\tlength: %g\n", tc->length);
}

//---------------------------------------------------------------

static void taiko_converter_slider_to_circles_normal(
    const struct taiko_converter *tc, struct osux_list *ho_list)
{    
    //fprintf(stderr, "to normal\n");
    if (tc->length <= tc->mpt) {
	// slider is too short
	struct hit_object * ho1 = ho_taiko_new(
            tc->ho->offset, tc->ho, 0);
	osux_list_append(ho_list, ho1);
	struct hit_object * ho2 = ho_taiko_new(
	    tc->ho->offset + tc->length, tc->ho, 1);
	osux_list_append(ho_list, ho2);
    } else {
	unsigned int i = 0;
	for (double offset = 0; offset_le(offset, tc->length); offset += tc->mpt) {
	    struct hit_object * ho = ho_taiko_new(
	        tc->ho->offset + offset, tc->ho, i % 2);
	    osux_list_append(ho_list, ho);
	    i++;
	}
    }
}

static void taiko_converter_slider_to_circles_repeat(
    const struct taiko_converter *tc, struct osux_list *ho_list)
{
    //fprintf(stderr, "to repeat\n");
    double unit = tc->length / tc->ho->sli.repeat;
    for (unsigned int i = 0; i <= tc->ho->sli.repeat; i++) {
	struct hit_object * ho = ho_taiko_new(
            tc->ho->offset + (i * unit), tc->ho, i);
	osux_list_append(ho_list, ho);
    }
}

//---------------------------------------------------------------

static void taiko_converter_convert_slider(
    const struct taiko_converter *tc, struct osux_list *ho_list)
{
    //taiko_converter_print(tc);
    if (tc->mpb * 2 <= tc->length) {
	//fprintf(stderr, "to slider\n");
	osux_list_append(ho_list, tc->ho);
    } else {
	if (tc->ho->sli.repeat != 1)
	    taiko_converter_slider_to_circles_repeat(tc, ho_list);
	else
	    taiko_converter_slider_to_circles_normal(tc, ho_list);
	ho_free(tc->ho);
    }
}

//---------------------------------------------------------------

static void taiko_converter_convert(
    const struct taiko_converter *tc, struct osux_list *ho_list)
{
    if (HO_IS_SPINNER(*tc->ho) || HO_IS_CIRCLE(*tc->ho))
	osux_list_append(ho_list, tc->ho);
    else if (HO_IS_SLIDER(*tc->ho))
	taiko_converter_convert_slider(tc, ho_list);
}

//---------------------------------------------------------------

static struct osux_list * taiko_autoconvert_ho_list(const osux_beatmap *bm)
{
    uint32_t current_tp = 0;
    struct osux_list * new_ho_list = osux_list_new(0);
    for (uint32_t i = 0; i < bm->hoc; ++i) {
	while(current_tp < (bm->tpc - 1) &&
	      bm->TimingPoints[current_tp + 1].offset
	      <= bm->HitObjects[i].offset)
	    current_tp++;

	struct taiko_converter * tc = taiko_converter_new(
            &bm->HitObjects[i], &bm->TimingPoints[current_tp],
	    bm->SliderMultiplier, bm->SliderTickRate);

	taiko_converter_convert(tc, new_ho_list);
	taiko_converter_free(tc);
    }
    return new_ho_list;
}

//---------------------------------------------------------------

static struct hit_object * osux_list_to_ho_array(const struct osux_list *l)
{
    unsigned int size = osux_list_size(l);
    struct hit_object * array = malloc(sizeof(*array) * size);
    for (unsigned int i = 0; i < size; ++i)
	array[i] = *(struct hit_object *) osux_list_get(l, i+1);
    return array;
}

//---------------------------------------------------------------

int osux_beatmap_taiko_autoconvert(osux_beatmap *bm)
{
    if (bm->Mode != MODE_STD)
	return -1;

    struct osux_list * new_ho_list = taiko_autoconvert_ho_list(bm);
    unsigned int hoc = osux_list_size(new_ho_list);
    struct hit_object * array = osux_list_to_ho_array(new_ho_list);

    osux_list_each(new_ho_list, (void (*)(void*)) ho_taiko_free);
    osux_list_free(new_ho_list);

    free(bm->HitObjects);
    bm->Mode       = MODE_TAIKO;
    bm->hoc        = hoc;
    bm->HitObjects = array;

    return 0;
}
