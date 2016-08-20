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
#include <string.h>
#include <glib.h>

#include "osux/hitobject_taiko.h"
#include "osux/hitsound.h"

osux_hitobject *osux_ho_taiko_circle_new(int offset, int sample)
{
    osux_hitobject *ho = g_malloc0(sizeof*ho);

    ho->offset = offset;
    ho->end_offset = offset;
    ho->type = HITOBJECT_CIRCLE;
    ho->hitsound.sample = sample;

    return ho;
}

osux_hitobject *osux_ho_taiko_spinner_new(int offset, int end_offset)
{
    osux_hitobject *ho = g_malloc0(sizeof*ho);

    ho->offset = offset;
    ho->end_offset = end_offset;
    ho->type = HITOBJECT_SPINNER | HITOBJECT_NEWCOMBO;
    ho->hitsound.sample = SAMPLE_NORMAL;

    return ho;
}

