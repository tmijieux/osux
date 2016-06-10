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

static inline GtsSurfaceInter *
tr_gts_surface_inter_new(GtsSurface * s1, GtsSurface * s2)
{
    return gts_surface_inter_new(gts_surface_inter_class(),
				 s1, s2,
				 gts_bb_tree_surface(s1),
				 gts_bb_tree_surface(s2),
				 0, 0);
}

static inline GtsSurface *
tr_gts_exclude_2_to_1(GtsSurface * s1, GtsSurface * s2)
{
    GtsSurfaceInter * si = tr_gts_surface_inter_new(s1, s2);
    GtsSurface * s = tr_gts_surface_new();
    gts_surface_inter_boolean(si, s, GTS_2_IN_1);
    gts_surface_inter_boolean(si, s, GTS_1_OUT_2);
    return s;
}

#endif //TR_GTS_H
