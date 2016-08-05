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

struct taiko_slider_converter {
    struct hit_object *ho;
    double mpb; // ms per beat
    double mpt; // ms per tick
    double length; // full length of the slider in ms, this include repetitions. Use double for more precision as a division is involved
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

// this function return the full length of the slider in ms, repetitions are included

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

static struct hit_object * ho_taiko_new(
    int offset, const struct hit_object *old_slider, 
    int edge_hs_index)
{
    struct hit_object *ho = calloc(sizeof(*ho), 1);

    // flag the object
    // taiko converter hitobjects are allocated while the beatmap
    // hitobjects aren't.
    // This will help us finding them in order to free them later.
    ho->spi.end_offset = -1;

    ho->offset = offset;
    ho->type   = HO_CIRCLE;
    // by default the hit_object inherit the slider global hitsound
    // when the slider has no additional hitsounds the global hitsound is kept
    ho->hs.sample = old_slider->hs.sample;

    if (old_slider->sli.hs.additional) {
	// if the slider has additional hitsounds the new circle inherit of one
	// of them.
	// Taiko use additional hitsounds located one the slider edges:
	// one at the start, one on each "repeat" and one at the end
	// Sometimes, the additional hitsound used may not be correspond the the 
	// circle offset. Mostly with simple sliders and high tick rate.

	// Example: 
	// with 1/4 (each character represent 1/4 beat) and tick rate = 4
	// Simple slider (does not repeat):
	// s----
	// Slider hitsound: (there are hitsound only at the start and the end)
	// d---k
	// Circle in taiko:
	// dkdkd
	// As you can see, the end circle is a don while the slider end 
	// hitsound is a kat.

	ho->hs.sample = old_slider->sli.hs.dat[edge_hs_index].sample;
    }

    return ho;
}

static void ho_taiko_free(struct hit_object * ho)
{
    if (ho->spi.end_offset == -1)
	free(ho);
}

//---------------------------------------------------------------

static void
taiko_slider_converter_init(struct taiko_slider_converter *tc,
			    struct hit_object *ho,
			    const struct timing_point *tp,
			    double sv, double tick_rate)
{
    tc->ho = ho;
    tc->mpb = tp->last_uninherited->mpb;
    tc->mpt = tc->mpb / tick_rate; // millisecond per (slider) tick
    tc->length = ho_slider_length(ho, tp, sv);
}

void taiko_slider_converter_print(struct taiko_slider_converter const *tc)
{
    fprintf(stderr, "taiko_slider_converter:\n");
    fprintf(stderr, "\toffset: %d\n", tc->ho->offset);
    fprintf(stderr, "\tmpb: %g\n", tc->mpb);
    fprintf(stderr, "\tmpt: %g\n", tc->mpt);
    fprintf(stderr, "\tlength: %g\n", tc->length);
}

//---------------------------------------------------------------

static void taiko_slider_converter_slider_to_circles_normal(
    const struct taiko_slider_converter *tc, struct osux_list *ho_list)
{
    if (tc->length <= tc->mpt) {
	// if slider length is lower than duration of 1 (slider) tick
        // then replace the slider by two circle:
        // positions are start and end of the old slider

        struct hit_object *ho1, *ho2;
	
	// start
	ho1 = ho_taiko_new(tc->ho->offset, tc->ho, 0);
	osux_list_append(ho_list, ho1);
	
	// end
        ho2 = ho_taiko_new(tc->ho->offset + tc->length, tc->ho, 1);
	osux_list_append(ho_list, ho2);

    } else {
        // else, the slider last for more than one tick.
	unsigned i;
        double offset;

	// add a circle for each tick, without exceeding slider length
	// But there is a small margin in the comparision, sliders are 
	// considered slightly longer than they are. 
	// It is quite hard to guess how peppy is computing this margin
	// so the margin used here is likely to be wrong. 
	// See offset_eq and offset_le for the margin computation.
	for (i = 0, offset = 0.;
             offset_le(offset, tc->length);
             offset += tc->mpt, ++i)
        {
	    struct hit_object *ho;

	    // The inherited hitsound is alterning between the start one (0) 
	    // and the end one (1). 
	    // Thus i%2 is used as the edge hitsound index.
            ho = ho_taiko_new(tc->ho->offset + offset, tc->ho, i % 2);
	    osux_list_append(ho_list, ho);
	}
    }
}

static void taiko_slider_converter_slider_to_circles_repeat(
    const struct taiko_slider_converter *tc, struct osux_list *ho_list)
{
    // compute the length in ms for the slider without repetitions
    double unit = tc->length / tc->ho->sli.repeat;

    // add one circle for each times the slider is repeated
    // circle are disposed with constant spacing ('unit')
    // so that the whole slider length is filled with circles.
    for (unsigned int i = 0; i <= tc->ho->sli.repeat; i++)
    {
	struct hit_object *ho;

	// the edge hitsound used is the one corresponding to the 
	// i-th repeat part.
        ho = ho_taiko_new(tc->ho->offset + (i * unit), tc->ho, i);
	osux_list_append(ho_list, ho);
    }
}

//---------------------------------------------------------------

static void taiko_slider_converter_convert(
    const struct taiko_slider_converter *tc, struct osux_list *ho_list)
{
    if (tc->length >= 2*tc->mpb)
        // if slider length if big enough, keep the slider
	osux_list_append(ho_list, tc->ho);
    else {
        // when the slider is too short, convert it to circles:
        // (two rules according to the slider being repeated or not)
	if (tc->ho->sli.repeat != 1)
	    taiko_slider_converter_slider_to_circles_repeat(tc, ho_list);
	else
	    taiko_slider_converter_slider_to_circles_normal(tc, ho_list);

	ho_free(tc->ho);
    }
}

//---------------------------------------------------------------

static struct osux_list * taiko_autoconvert_ho_list(const osux_beatmap *bm)
{
    uint32_t current_tp = 0;
    struct osux_list *new_ho_list = osux_list_new(0);

    for (uint32_t i = 0; i < bm->hoc; ++i) {
	struct hit_object * ho = &bm->HitObjects[i];

        // for each hit object
        // compute timing point applying on this hit object
	while (current_tp < (bm->tpc - 1) &&
	       bm->TimingPoints[current_tp + 1].offset <= ho->offset)
	    current_tp++;

	struct timing_point * tp = &bm->TimingPoints[current_tp];

	if (HO_IS_SPINNER(*ho) || HO_IS_CIRCLE(*ho)) {
	    // keep spinner and circle
	    osux_list_append(new_ho_list, ho);
	} else if (HO_IS_SLIDER(*ho)) {
	    // build convert helper
	    struct taiko_slider_converter tc;
	    taiko_slider_converter_init(
		&tc, ho, tp,
		bm->SliderMultiplier, bm->SliderTickRate);

	    // convert the slider to circles
	    // the result hit objects are appended into new_ho_list
	    taiko_slider_converter_convert(&tc, new_ho_list);
	}
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

    osux_list_each(new_ho_list, (void (*)(void*))ho_taiko_free);
    osux_list_free(new_ho_list);

    free(bm->HitObjects);
    bm->Mode       = MODE_TAIKO;
    bm->hoc        = hoc;
    bm->HitObjects = array;

    return 0;
}
