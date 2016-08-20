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

#include <glib.h>

#include "osux/hitsound.h"
#include "osux/hitobject.h"
#include "osux/hitobject_taiko.h"

int osux_ho_taiko_circle_init(osux_hitobject *ho, int offset, int sample)
{
    int err = 0;
    char *rep = NULL;
    rep = g_strdup_printf(
        "0,0,%d,%d,%d,0:0:0:0:", offset, HITOBJECT_CIRCLE, sample);
    err = osux_hitobject_init(ho, rep, 12);
    g_free(rep);
    return err;
}

int osux_ho_taiko_spinner_init(osux_hitobject *ho, int offset, int end_offset)
{
    int err = 0;
    char *rep = NULL;
    rep = g_strdup_printf(
        "0,0,%d,%d,%d,%d,0:0:0:0:", offset,
        HITOBJECT_SPINNER | HITOBJECT_NEWCOMBO,  SAMPLE_NORMAL, end_offset);
    err = osux_hitobject_init(ho, rep, 12);
    g_free(rep);
    return err;
}
