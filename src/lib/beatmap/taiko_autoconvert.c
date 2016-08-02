#ifndef OSUX_TAIKO_AUTOCONVERT_H
#define OSUX_TAIKO_AUTOCONVERT_H
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
    double length; // which length is this? (seems to be time length?)
};


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


// this function return the length of the slider on the x'th dimension
// (ORIGINAL AUTHOR INSERT DIMENSION HERE) (this seems to be 4 though)
static double ho_slider_length(
    const struct hit_object *ho, // the slider
    const struct timing_point *tp, // timing_point applying on the slider
    double sv // global beatmap slider velocity
)
{
    if (!tp->uninherited)
	sv *= -100. / tp->svm; // we should thanks peppy for this
    // in an ideal world this would have been:
    // sv *= tp->svm;

    return (ho->sli.length * ho->sli.repeat * tp->last_uninherited->mpb)
        / (100 * sv);
}

//---------------------------------------------------------------

// return the newly created taiko circle!

static struct hit_object *
ho_taiko_new(
    double offset, // isn't double only for TimingPoint's offsets?
    const struct hit_object *old_slider,
    int add_index// in the future this will be called edge_hitsound_index
    // or slider_tick_hitsound_index
)
{
    struct hit_object *ho = calloc(sizeof(*ho), 1);

    ho->offset = offset;
    ho->type   = HO_CIRCLE;
    ho->hs.sample = old_slider->hs.sample;

    if (old_slider->sli.hs.additional){
        // this has to do with slider tick Sample
        // according to the convert rule, and if custom sample are present on
        // slider ticks, newly created circle will inherits from
        // the tick Sample (i.e in Taiko, this can change color and size of circles)
        // BUT THIS OBVIOUSLY NEED MOOAARR COMMENT FROM THE ORIGINAL AUTHOR
	ho->hs.sample = old_slider->sli.hs.dat[add_index].sample;
    }

    return ho;
}

//---------------------------------------------------------------

static void
taiko_converter_init(struct taiko_converter *tc,
                     struct hit_object *ho,
                     const struct timing_point *tp,
                     double sv, double tick_rate)
{
    tc->ho = ho;
    tc->mpb = tp->last_uninherited->mpb;
    tc->mpt = tc->mpb / tick_rate; // millisecond per (slider) tick

    if (HO_IS_SLIDER(*ho))
	tc->length = ho_slider_length(ho, tp, sv);
    else
	tc->length = 0;// if we call this class slider_converter we can remove this line ;)
}

void taiko_converter_print(struct taiko_converter const *tc)
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
    if (tc->length <= tc->mpt) {
	// if slider length is lower than duration of 1 (slider) tick

        // then replace the slider by two circle:
        // positions are start and end of the old slider

        struct hit_object *ho1, *ho2;
                            /* begin */
	ho1 = ho_taiko_new(tc->ho->offset, tc->ho, 0);
        ho2 = ho_taiko_new(tc->ho->offset + tc->length, tc->ho, 1);
                                       /* end */
	osux_list_append(ho_list, ho1);
	osux_list_append(ho_list, ho2);

    } else {
        // else, the slider last for more than one tick.
	unsigned i;
        double offset;

	for (i = 0, offset = 0.;
             offset_le(offset, tc->length);
             offset += tc->mpt, ++i)
        {
	    struct hit_object *ho;

            // add a circle for each tick, without exceeding slider length
            ho = ho_taiko_new(tc->ho->offset + offset, tc->ho, i % 2);
	    osux_list_append(ho_list, ho);
	}
    }
}

static void taiko_converter_slider_to_circles_repeat(
    const struct taiko_converter *tc, struct osux_list *ho_list)
{
    double unit = tc->length / tc->ho->sli.repeat;

    // if the slider is repeated
    for (unsigned int i = 0; i <= tc->ho->sli.repeat; i++)
    {
	struct hit_object *ho;
        // add one circle for each times the slider is repeated
        // circle are disposed with constant spacing ('unit')
        // so that the whole slider length is filled with circles.

        ho = ho_taiko_new(tc->ho->offset + (i * unit), tc->ho, i);
	osux_list_append(ho_list, ho);
    }
}

//---------------------------------------------------------------

static void taiko_converter_convert_slider(
    const struct taiko_converter *tc, struct osux_list *ho_list)
{
    if (tc->length >= 2*tc->mpb)
        // if slider length if big enough, keep the slider
	osux_list_append(ho_list, tc->ho);
    else {
        // when the slider is too short, convert it to circles:
        // (two rules according to the slider being repeated or not)
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
        // keep spinner and circle
	osux_list_append(ho_list, tc->ho);

    else if (HO_IS_SLIDER(*tc->ho))
        // for circle keep or replace by circle according to some conditions
	taiko_converter_convert_slider(tc, ho_list);
}

//---------------------------------------------------------------

static struct osux_list * taiko_autoconvert_ho_list(const osux_beatmap *bm)
{
    uint32_t current_tp = 0;
    struct osux_list *new_ho_list = osux_list_new(0);

    for (uint32_t i = 0; i < bm->hoc; ++i) {

        // for each hit object
        // compute timing point applying on this hit object

	while (current_tp < (bm->tpc - 1) &&
	      bm->TimingPoints[current_tp + 1].offset
	      <= bm->HitObjects[i].offset)
	    current_tp++;

        // build convert helper

        struct taiko_converter tc;
        taiko_converter_init(
            &tc, &bm->HitObjects[i], &bm->TimingPoints[current_tp],
	    bm->SliderMultiplier, bm->SliderTickRate);

        // convert (keep hit object or replace slider by
        // circle under some conditions)
	taiko_converter_convert(&tc, new_ho_list);
        // the result hit objects are appended into new_ho_list
    }
    return new_ho_list;
}

//---------------------------------------------------------------

static struct hit_object *osux_list_to_ho_array(const struct osux_list *l)
{
    struct hit_object *array;
    unsigned int size = osux_list_size(l);
    array = malloc(sizeof(*array) * size);

    for (unsigned int i = 0; i < size; ++i)
	array[i] = *(struct hit_object*) osux_list_get(l, i+1);

    return array;
}

int osux_beatmap_taiko_autoconvert(osux_beatmap *bm)
{
    if (bm->Mode != MODE_STD)
	return -1;

    struct osux_list *new_ho_list;
    struct hit_object *array;
    unsigned hoc;

    new_ho_list = taiko_autoconvert_ho_list(bm);
    hoc = osux_list_size(new_ho_list);
    array = osux_list_to_ho_array(new_ho_list);

    osux_list_each(new_ho_list, free);
    osux_list_free(new_ho_list);

    free(bm->HitObjects);
    bm->Mode       = MODE_TAIKO;
    bm->hoc        = hoc;
    bm->HitObjects = array;

    return 0;
}

#endif // OSUX_TAIKO_AUTOCONVERT_H
