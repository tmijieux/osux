/*
 *  COPYRIGHT (©) 2015-2016 Lucas Maugère, Thomas Mijieux
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

#include "taiko_ranking_object.h"
#include "tr_gts.h"

GtsSurface * tr_gts_exclude_2_to_1(GtsSurface * s1, GtsSurface * s2)
{
    GtsSurfaceInter * si = tr_gts_surface_inter_new(s1, s2);
    GtsSurface * s = tr_gts_surface_new();
    gts_surface_inter_boolean(si, s, GTS_1_OUT_2);
    gts_surface_inter_boolean(si, s, GTS_2_IN_1);
    gts_surface_foreach_face(si->s2, (GtsFunc) gts_triangle_revert, NULL);
    gts_surface_foreach_face(s2, (GtsFunc) gts_triangle_revert, NULL);
    return s;
}


