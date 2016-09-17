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
#ifndef TR_GTS_H
#define TR_GTS_H

static inline GtsVertex *
tr_gts_vertex_new(int offset, double objs, double interest)
{
    return gts_vertex_new(gts_vertex_class(), offset, objs, interest);
}

// Always set bottom vertex as first (if there is a bottom vertex...)
static inline GtsEdge *
tr_gts_edge_new(GtsVertex * v_bot, GtsVertex * v_top)
{
    return gts_edge_new(gts_edge_class(), v_bot, v_top);
}

static inline GtsFace *
tr_gts_face_new(GtsEdge * e1, GtsEdge * e2, GtsEdge * e3)
{
    return gts_face_new(gts_face_class(), e1, e2, e3);
}

static inline GtsSurface * tr_gts_surface_new(void)
{
    return gts_surface_new(gts_surface_class(),
                           gts_face_class(),
                           gts_edge_class(),
                           gts_vertex_class());
}

static inline GtsSurface * tr_gts_surface_copy(GtsSurface * s)
{
    GtsSurface * copy = tr_gts_surface_new();
    return gts_surface_copy(copy, s);
}

static inline GtsSurfaceInter *
tr_gts_surface_inter_new(GtsSurface * s1, GtsSurface * s2)
{
    GNode * t1 = gts_bb_tree_surface(s1);
    GNode * t2 = gts_bb_tree_surface(s2);
    return gts_surface_inter_new(gts_surface_inter_class(),
                                 s1, s2, t1, t2, 0, 0);
}

GtsSurface * tr_gts_exclude_2_to_1(GtsSurface * s1, GtsSurface * s2);

#endif //TR_GTS_H
