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
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "osux.h"

#include "bpm.h"
#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "cst_yaml.h"
#include "linear_fun.h"
#include "vector.h"
#include "print.h"
#include "tr_gts.h"
#include "reading.h"

// Pretty much the location in my head...
struct edge_rect {
    GtsEdge *e_bot;
    GtsEdge *e_top;
    GtsEdge *e_back;
    GtsEdge *e_front;

    GtsVertex *v_back_bot;
    GtsVertex *v_back_top;
    GtsVertex *v_front_bot;
    GtsVertex *v_front_top;
};

struct edge_2_rect {
    struct edge_rect *old;
    struct edge_rect *new;

    GtsEdge *e_front_bot;
    GtsEdge *e_front_top;
    GtsEdge *e_back_bot;
    GtsEdge *e_back_top;
};

//--------------------------------------------------

static struct yaml_wrap *yw_rdg;
static osux_hashtable *ht_cst_rdg;

static double tro_seen(const struct tr_object *o);
static struct table *
tro_get_obj_hiding(const struct tr_object *o, int i);

static void trm_set_app_dis_offset(struct tr_map *map);
static void trm_set_line_coeff(struct tr_map *map);
static void trm_set_mesh(struct tr_map *map);
static void trm_set_seen(struct tr_map *map);
static void trm_free_mesh(struct tr_map *map);
static void trm_set_obj_hiding(struct tr_map *map);
static void trm_free_obj_hiding(struct tr_map *map);
static void trm_set_reading_star(struct tr_map *map);

//--------------------------------------------------

#define READING_FILE "reading_cst.yaml"

static struct linear_fun *SEEN_LF;
static struct linear_fun *INTEREST_LF;
static struct vector *INTEREST_VECT;

// coeff for star
static double READING_STAR_COEFF_SEEN;
static struct linear_fun *READING_SCALE_LF;

//-----------------------------------------------------

static void reading_global_init(osux_hashtable *ht_cst)
{
    INTEREST_VECT = cst_vect(ht_cst, "vect_interest");
    INTEREST_LF = cst_lf(ht_cst, "vect_interest");
    SEEN_LF  = cst_lf(ht_cst, "vect_seen");
    READING_SCALE_LF = cst_lf(ht_cst, "vect_reading_scale");

    READING_STAR_COEFF_SEEN = cst_f(ht_cst, "star_seen");
}

//-----------------------------------------------------

static void ht_cst_exit_reading(void)
{
    yaml2_free(yw_rdg);
    vect_free(INTEREST_VECT);
    lf_free(INTEREST_LF);
    lf_free(SEEN_LF);
    lf_free(READING_SCALE_LF);
}

void tr_reading_initialize(void)
{
    yw_rdg = cst_get_yw(READING_FILE);
    ht_cst_rdg = yw_extract_ht(yw_rdg);
    if (ht_cst_rdg != NULL)
        reading_global_init(ht_cst_rdg);
    atexit(ht_cst_exit_reading);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

enum done_offset {
    OFFSET_APP     = 1 << 0,
    END_OFFSET_APP = 1 << 1,
    OFFSET_DIS     = 1 << 2,
    END_OFFSET_DIS = 1 << 3
};

static inline int tro_is_done(struct tr_object *o, enum done_offset d)
{
    return o->done & d;
}

static inline void tro_set_done(struct tr_object *o, enum done_offset d)
{
    o->done |= d;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static inline GtsVertex *
tr_gts_vertex_top_new(int offset, double objs, struct tr_object *o)
{
    int diff = o->end_offset_dis_2 - offset;
    double z = lf_eval(INTEREST_LF, diff);
    return tr_gts_vertex_new(offset, objs, z);
}

//-----------------------------------------------------

static inline GtsVertex *
tr_gts_vertex_bot_new(int offset, double objs)
{
    return tr_gts_vertex_new(offset, objs, 0);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_eval_obj_front(struct tr_object *o, int offset)
{
    if (o->offset_dis <= offset && offset <= o->end_offset_dis)
        return o->obj_dis;
    else if (o->offset_app <= offset)
        return o->line_a * offset + o->line_b;
    else {
        tr_error("Out of bounds for tro_eval_obj_front: %d", offset);
        tro_print(o, FILTER_BASIC | FILTER_READING_PLUS);
    }
    return -1;
}

//-----------------------------------------------------

static double tro_eval_obj_back(struct tr_object *o, int offset)
{
    if (o->offset_app <= offset && offset <= o->end_offset_app)
        return o->obj_app;
    else if (offset <= o->end_offset_dis)
        return o->line_a * offset + o->line_b_end;
    else {
        tr_error("Out of bounds for tro_eval_obj_back: %d", offset);
        tro_print(o, FILTER_BASIC | FILTER_READING_PLUS);
    }
    return -1;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void e2r_link(struct edge_2_rect *r)
{
    r->e_front_bot = tr_gts_edge_new(r->old->v_front_bot,
                                     r->new->v_front_bot);
    r->e_front_top = tr_gts_edge_new(r->old->v_front_top,
                                     r->new->v_front_top);
    r->e_back_bot  = tr_gts_edge_new(r->old->v_back_bot,
                                     r->new->v_back_bot);
    r->e_back_top  = tr_gts_edge_new(r->old->v_back_top,
                                     r->new->v_back_top);
}

//-----------------------------------------------------

static struct edge_2_rect *
e2r_rect_link_vertex(struct edge_rect *old, GtsVertex *v_new)
{
    struct edge_2_rect *r = malloc(sizeof(*r));
    r->old = old;

    r->new = malloc(sizeof(*(r->new)));
    r->new->e_front = NULL;
    r->new->e_back  = NULL;
    r->new->e_top   = NULL;
    r->new->e_bot   = NULL;

    r->new->v_front_bot = v_new;
    r->new->v_back_bot  = v_new;
    r->new->v_front_top = v_new;
    r->new->v_back_top  = v_new;

    e2r_link(r);
    return r;
}

//-----------------------------------------------------

static struct edge_2_rect *
e2r_vertex_link_rect(GtsVertex *v_old, struct edge_rect *new)
{
    struct edge_2_rect *r = malloc(sizeof(*r));
    r->new = new;

    r->old = malloc(sizeof(*(r->old)));
    r->old->e_front = NULL;
    r->old->e_back  = NULL;
    r->old->e_top   = NULL;
    r->old->e_bot   = NULL;

    r->old->v_front_bot = v_old;
    r->old->v_back_bot  = v_old;
    r->old->v_front_top = v_old;
    r->old->v_back_top  = v_old;

    e2r_link(r);
    return r;
}

//-----------------------------------------------------

static struct edge_2_rect *
e2r_rect_link_edge(struct edge_rect *old, GtsEdge *e_new)
{
    struct edge_2_rect *r = malloc(sizeof(*r));
    r->old = old;

    r->new = malloc(sizeof(*(r->new)));
    r->new->e_front = e_new;
    r->new->e_back  = e_new;
    r->new->e_top   = NULL;
    r->new->e_bot   = NULL;

    r->new->v_front_bot = e_new->segment.v1;
    r->new->v_back_bot  = e_new->segment.v1;
    r->new->v_front_top = e_new->segment.v2;
    r->new->v_back_top  = e_new->segment.v2;

    e2r_link(r);
    return r;
}

//-----------------------------------------------------

static struct edge_2_rect *
e2r_edge_link_rect(GtsEdge *e_old, struct edge_rect *new)
{
    struct edge_2_rect *r = malloc(sizeof(*r));
    r->new = new;

    r->old = malloc(sizeof(*(r->old)));
    r->old->e_front = e_old;
    r->old->e_back  = e_old;
    r->old->e_top   = NULL;
    r->old->e_bot   = NULL;

    r->old->v_front_bot = e_old->segment.v1;
    r->old->v_back_bot  = e_old->segment.v1;
    r->old->v_front_top = e_old->segment.v2;
    r->old->v_back_top  = e_old->segment.v2;

    e2r_link(r);
    return r;
}

//-----------------------------------------------------

static struct edge_2_rect *
e2r_rect_link_rect(struct edge_rect *old, struct edge_rect *new)
{
    struct edge_2_rect *r = malloc(sizeof(*r));
    r->old = old;
    r->new = new;

    e2r_link(r);
    return r;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static int mesh_has_volume(GtsSurface *mesh)
{
    int has = 1;
    if (!gts_surface_is_closed(mesh)) {
        tr_error("Mesh is not closed!");
        has = 0;
    }
    if (!gts_surface_is_manifold(mesh)) {
        tr_error("Mesh is not manifold!");
        has = 0;
    }
    if (!gts_surface_is_orientable(mesh)) {
        tr_error("Mesh is not orientable!");
        has = 0;
    }
    return has;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void tro_mark_mesh_offset(struct tr_object *o,
                                 int offset, int max)
{
    if (max == offset)
        o->count++;
    if (!tro_is_done(o, END_OFFSET_APP) && max == o->end_offset_app)
        tro_set_done(o, END_OFFSET_APP);
    if (!tro_is_done(o, OFFSET_DIS)     && max == o->offset_dis)
        tro_set_done(o, OFFSET_DIS);
    if (!tro_is_done(o, END_OFFSET_DIS) && max == o->end_offset_dis)
        tro_set_done(o, END_OFFSET_DIS);
    if (max <= o->offset_app)
        tro_set_done(o, OFFSET_APP);
}

//-----------------------------------------------------

static int tro_get_next_mesh_offset(struct tr_object *o)
{
    // If the interest value is finished mark the object as done and
    // return
    if (o->count >= INTEREST_VECT->len) {
        tro_set_done(o, OFFSET_APP);
        return 0;
    }

    // next offset where the interest value change of slope.
    int offset = o->end_offset_dis_2 - INTEREST_VECT->t[o->count][0];


    int max;
    if (!tro_is_done(o, END_OFFSET_DIS) &&
        o->end_offset_dis != o->end_offset_dis_2) { // when starting HD
        max = o->end_offset_dis;
        while (offset > max) {
            o->count++;
            offset = o->end_offset_dis_2 - INTEREST_VECT->t[o->count][0];
        }
    }

    if (!tro_is_done(o, END_OFFSET_APP) && !tro_is_done(o, OFFSET_DIS))
        max = max(offset, max(o->end_offset_app, o->offset_dis));
    else if (tro_is_done(o, END_OFFSET_APP) && !tro_is_done(o, OFFSET_DIS))
        max = max(offset, o->offset_dis);
    else if (!tro_is_done(o, END_OFFSET_APP) && tro_is_done(o, OFFSET_DIS))
        max = max(offset, o->end_offset_app);
    else
        max = offset;
    tro_mark_mesh_offset(o, offset, max);
    return max;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static struct edge_rect *tro_get_rect(struct tr_object *o, int offset)
{
    struct edge_rect *r = malloc(sizeof(*r));

    double objs_front = tro_eval_obj_front(o, offset);
    r->v_front_bot = tr_gts_vertex_bot_new(offset, objs_front);
    r->v_front_top = tr_gts_vertex_top_new(offset, objs_front, o);

    double objs_back  = tro_eval_obj_back(o, offset);
    r->v_back_bot = tr_gts_vertex_bot_new(offset, objs_back);
    r->v_back_top = tr_gts_vertex_top_new(offset, objs_back, o);

    r->e_front = tr_gts_edge_new(r->v_front_bot, r->v_front_top);
    r->e_back = tr_gts_edge_new(r->v_back_bot, r->v_back_top);
    r->e_top = tr_gts_edge_new(r->v_back_top, r->v_front_top);
    r->e_bot = tr_gts_edge_new(r->v_back_bot, r->v_front_bot);

    return r;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void tro_add_face_front(struct tr_object *o,
                               struct edge_2_rect *r)
{
    GtsEdge *e_diag = tr_gts_edge_new(r->old->v_front_top,
                                      r->new->v_front_bot);

    GtsFace *f1 = tr_gts_face_new(r->old->e_front,
                                  e_diag,
                                  r->e_front_bot);
    GtsFace *f2 = tr_gts_face_new(e_diag,
                                  r->e_front_top,
                                  r->new->e_front);
    gts_surface_add_face(o->mesh, f1);
    gts_surface_add_face(o->mesh, f2);
}

//-----------------------------------------------------

static void tro_add_face_back(struct tr_object *o,
                              struct edge_2_rect *r)
{
    GtsEdge *e_diag = tr_gts_edge_new(r->old->v_back_top,
                                      r->new->v_back_bot);

    GtsFace *f1 = tr_gts_face_new(r->old->e_back,
                                  r->e_back_bot,
                                  e_diag);
    GtsFace *f2 = tr_gts_face_new(e_diag,
                                  r->new->e_back,
                                  r->e_back_top);
    gts_surface_add_face(o->mesh, f1);
    gts_surface_add_face(o->mesh, f2);
}

//-----------------------------------------------------

static void tro_add_face_top(struct tr_object *o,
                             struct edge_2_rect *r)
{
    GtsEdge *e_diag = tr_gts_edge_new(r->old->v_back_top,
                                      r->new->v_front_top);

    GtsFace *f1 = tr_gts_face_new(r->old->e_top,
                                  e_diag,
                                  r->e_front_top);
    GtsFace *f2 = tr_gts_face_new(e_diag,
                                  r->e_back_top,
                                  r->new->e_top);
    gts_surface_add_face(o->mesh, f1);
    gts_surface_add_face(o->mesh, f2);
}

//-----------------------------------------------------

static void tro_add_face_bot(struct tr_object *o,
                             struct edge_2_rect *r)
{
    GtsEdge *e_diag = tr_gts_edge_new(r->old->v_back_bot,
                                      r->new->v_front_bot);

    GtsFace *f1 = tr_gts_face_new(r->old->e_bot,
                                  r->e_front_bot,
                                  e_diag);
    GtsFace *f2 = tr_gts_face_new(e_diag,
                                  r->new->e_bot,
                                  r->e_back_bot);
    gts_surface_add_face(o->mesh, f1);
    gts_surface_add_face(o->mesh, f2);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static struct edge_2_rect *
tro_open_front_back_mesh(struct tr_object *o, int offset)
{
    GtsVertex *v_top = tr_gts_vertex_top_new(o->end_offset_dis, o->obj_dis, o);
    struct edge_rect *r_new = tro_get_rect(o, offset);
    struct edge_2_rect *r;
    if (v_top->p.z == 0) {
        r = e2r_vertex_link_rect(v_top, r_new);
        GtsFace *f_front = tr_gts_face_new(
            r->new->e_front, r->e_front_bot, r->e_front_top);
        GtsFace *f_back  = tr_gts_face_new(
            r->new->e_back,  r->e_back_top,  r->e_back_bot);
        gts_surface_add_face(o->mesh, f_front);
        gts_surface_add_face(o->mesh, f_back);
    } else {
        GtsVertex *v_bot = tr_gts_vertex_bot_new(
            o->end_offset_dis, o->obj_dis);
        GtsEdge *e = tr_gts_edge_new(v_bot, v_top);
        r = e2r_edge_link_rect(e, r_new);
        tro_add_face_front(o, r);
        tro_add_face_back(o, r);
    }
    return r;
}

//-----------------------------------------------------

static struct edge_rect *tro_open_mesh(struct tr_object *o)
{
    int offset = tro_get_next_mesh_offset(o);
    if (tro_is_done(o, OFFSET_APP))
        return NULL;

    struct edge_2_rect *r = tro_open_front_back_mesh(o, offset);

    GtsFace *f_top = tr_gts_face_new(
        r->new->e_top, r->e_front_top, r->e_back_top);
    GtsFace *f_bot = tr_gts_face_new(
        r->new->e_bot, r->e_back_bot,  r->e_front_bot);
    gts_surface_add_face(o->mesh, f_top);
    gts_surface_add_face(o->mesh, f_bot);

    struct edge_rect *r_new = r->new;
    free(r->old);
    free(r);
    return r_new;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static struct edge_2_rect *
tro_close_front_back_mesh(struct tr_object *o, struct edge_rect *r_old)
{
    GtsVertex *v_top = tr_gts_vertex_top_new(o->offset_app, o->obj_app, o);
    struct edge_2_rect *r;
    if (v_top->p.z == 0) {
        r = e2r_rect_link_vertex(r_old, v_top);
        GtsFace *f_front = tr_gts_face_new(
            r->old->e_front, r->e_front_top, r->e_front_bot);
        GtsFace *f_back  = tr_gts_face_new(
            r->old->e_back,  r->e_back_bot,  r->e_back_top);
        gts_surface_add_face(o->mesh, f_front);
        gts_surface_add_face(o->mesh, f_back);
    } else {
        GtsVertex *v_bot = tr_gts_vertex_bot_new(o->offset_app, o->obj_app);
        GtsEdge *e = tr_gts_edge_new(v_bot, v_top);
        r = e2r_rect_link_edge(r_old, e);
        tro_add_face_front(o, r);
        tro_add_face_back(o, r);
    }
    return r;
}

//-----------------------------------------------------

static void tro_close_mesh(struct tr_object *o,
                           struct edge_rect *r_old)
{
    struct edge_2_rect *r = tro_close_front_back_mesh(o, r_old);

    GtsFace *f_top = tr_gts_face_new(
        r->old->e_top, r->e_back_top,  r->e_front_top);
    GtsFace *f_bot = tr_gts_face_new(
        r->old->e_bot, r->e_front_bot, r->e_back_bot);
    gts_surface_add_face(o->mesh, f_top);
    gts_surface_add_face(o->mesh, f_bot);

    free(r->old);
    free(r->new);
    free(r);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static struct edge_rect *
tro_adv_mesh_step(struct tr_object *o,int offset, struct edge_rect *r_old)
{
    struct edge_rect *r_new = tro_get_rect(o, offset);
    struct edge_2_rect *r = e2r_rect_link_rect(r_old, r_new);

    tro_add_face_front(o, r);
    tro_add_face_back(o, r);
    tro_add_face_top(o, r);
    tro_add_face_bot(o, r);

    free(r_old);
    free(r);
    return r_new;
}

//-----------------------------------------------------

static struct edge_rect *tro_adv_mesh(struct tr_object *o,
                                       struct edge_rect *rect)
{
    struct edge_rect *r_old = rect;

    while (1) {
        int offset = tro_get_next_mesh_offset(o);
        if (tro_is_done(o, OFFSET_APP))
            break;
        r_old = tro_adv_mesh_step(o, offset, r_old);
    }
    return r_old;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_seen(const struct tr_object *o)
{
    if (!mesh_has_volume(o->mesh)) {
        tr_error("Mesh does not have a volume!");
        gts_surface_print_stats(o->mesh, stderr);
    }
    if (o->line_a == INFINITY)
        return lf_eval(SEEN_LF, 0); // if the object has an insane bpma

    double seen = gts_surface_volume(o->final_mesh);

    // Add the object height dimension
    seen *= tro_get_radius(o);
    return lf_eval(SEEN_LF, seen);
}

//-----------------------------------------------------

static struct table *
tro_get_obj_hiding(const struct tr_object *o, int i)
{
    // list object that hide the i-th
    struct table *obj_h = table_new(i);

    for (int j = 0; j < i; j++) {
        if (o->objs[j].ps == MISS)
            continue;
        // if i has appeared before j
        if (o->objs[j].end_offset_app - o->offset_app > 0)
            table_add(obj_h, &o->objs[j]);
    }
    return obj_h;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void tro_set_line_coeff(struct tr_object *o)
{
    o->line_a = ((o->obj_app    - o->obj_dis) /
                 (o->offset_app - o->offset_dis));
    o->line_b     = o->obj_app - o->line_a * o->offset_app;
    o->line_b_end = o->obj_app - o->line_a * o->end_offset_app;
}

//-----------------------------------------------------

/**
 * Change 'o' offset according to same bpm object that are hiding it.
 * And set them to NULL
 */
void tro_set_app_dis_offset_same_bpm(struct tr_object *o)
{
    for (int i = 0; i < table_len(o->obj_h); i++) {
        const struct tr_object *o2 = table_get(o->obj_h, i);
        if (equal(o->bpm_app, o2->bpm_app)) {
            if (o->offset_app < o2->offset_dis) {
                o->offset_app     = o2->offset_dis;
                o->end_offset_app = o2->end_offset_dis;
            }
            table_set(o->obj_h, i, NULL);
        }
    }
}

//-----------------------------------------------------

void tro_set_app_dis_offset(struct tr_object *o)
{
    double space_unit = mpb_to_bpm(o->bpm_app) / 4.;
    double radius = tro_get_radius(o);

    o->offset_app = (o->offset -
                     (o->obj_app + radius) * space_unit);
    o->offset_dis = (o->end_offset -
                     (o->obj_dis + radius) * space_unit);

    o->end_offset_app = (o->offset -
                         (o->obj_app - radius) * space_unit);
    o->end_offset_dis = (o->end_offset -
                         (o->obj_dis - radius) * space_unit);

    o->end_offset_dis_2 = o->end_offset + radius * space_unit;
}

//-----------------------------------------------------

/*
 * Creating the mesh in three dimension:
 * x = time in ms
 * y = object width reprenseted by the object borders positions in
 * number of objects positionable without superposition
 * before.
 * z = interest
 * Example for y:
 * 1/4 stream, "|" are the border of the screen
 * |ddddkkkkddddkkkk|
 * ^ object to play
 * The first kat as 4 objects positionable without superposition
 * on its left border and 5 objects positionable without
 * superposition on its right border.
 */
void tro_set_mesh_base(struct tr_object *o)
{
    o->mesh = tr_gts_surface_new();
    o->count = 0;
    o->done = 0;

    struct edge_rect *rect = tro_open_mesh(o);
    if (rect == NULL) {
        tr_warning("Can't open mesh...");
        return;
    }
    rect = tro_adv_mesh(o, rect);
    tro_close_mesh(o, rect);
    // gts_surface_print_stats(o->mesh, stderr);
}

//-----------------------------------------------------

void tro_set_mesh_remove_intersection(struct tr_object *o)
{
    GtsSurface *mesh = tr_gts_surface_copy(o->mesh);
    /* Not working for now
    for (int i = 0; i < table_len(o->obj_h); i++) {
        const struct tr_object *o2 = table_get(o->obj_h, i);
        if (o2 == NULL) // already done with same bpm
            continue;
        GtsSurface *mesh2 = tr_gts_surface_copy(o2->mesh);
        mesh = tr_gts_exclude_2_to_1(mesh, mesh2);
    }
    */
    o->final_mesh = mesh;
}

//-----------------------------------------------------

void tro_free_mesh(struct tr_object *o)
{
    gts_object_destroy(GTS_OBJECT(o->mesh));
    gts_object_destroy(GTS_OBJECT(o->final_mesh));
}

//-----------------------------------------------------

void tro_set_seen(struct tr_object *o)
{
    if (o->ps == MISS || o->obj_app <= o->obj_dis) {
        o->seen = lf_eval(SEEN_LF, 0);
        return;
    }
    o->seen = tro_seen(o);
}

//-----------------------------------------------------

void tro_set_obj_hiding(struct tr_object *o, int i)
{
    o->obj_h = tro_get_obj_hiding(o, i);
}

//-----------------------------------------------------

void tro_free_obj_hiding(struct tr_object *o)
{
    table_free(o->obj_h);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_set_line_coeff(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_set_line_coeff(&map->object[i]);
}

//-----------------------------------------------------

static void trm_set_app_dis_offset(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_set_app_dis_offset(&map->object[i]);
}

//-----------------------------------------------------

static void trm_set_app_dis_offset_same_bpm(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_set_app_dis_offset_same_bpm(&map->object[i]);
}

//-----------------------------------------------------

static void trm_set_seen(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_set_seen(&map->object[i]);
}

//-----------------------------------------------------

static void trm_set_mesh(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_set_mesh_base(&map->object[i]);
    for (int i = 0; i < map->nb_object; i++)
        tro_set_mesh_remove_intersection(&map->object[i]);
}

//-----------------------------------------------------

static void trm_free_mesh(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_free_mesh(&map->object[i]);
}

//-----------------------------------------------------

static void trm_set_obj_hiding(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_set_obj_hiding(&map->object[i], i);
}

//-----------------------------------------------------

static void trm_free_obj_hiding(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_free_obj_hiding(&map->object[i]);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void tro_set_reading_star(struct tr_object *obj)
{
    obj->reading_star = lf_eval
        (READING_SCALE_LF, READING_STAR_COEFF_SEEN * obj->seen);
}

//-----------------------------------------------------

static void trm_set_reading_star(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_set_reading_star(&map->object[i]);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_reading(struct tr_map *map)
{
    if (ht_cst_rdg == NULL) {
        tr_error("Unable to compute reading stars.");
        return;
    }

    /*
      Computation rely on how long and which portion of the object
      is seen.
      Four dimensions are used for this:
      - object width
      - object height
      - time
      - interest
      Width and height are used as if the object was a square because
      it maximizes its visibility.

      Object width:
      The hit object width, this value is not constant as objects may
      be superposed. For example with a regular slider velocity
      objects in 1/6 are superposed and therefore less visible.

      Object height:
      The hit object height. As this value is constant it is not
      included in the mesh and is added at the end.
      See tro_seen().

      Time:
      The object visibility depends obviously depends on time.
      Starting when the object is on the right of the screen and
      ending when it is on the left.

      Interest:
      Seeing the object is not always useful. Watching it just before
      playing it does not help a lot.
     */
    trm_set_app_dis_offset(map);
    trm_set_obj_hiding(map);
    trm_set_app_dis_offset_same_bpm(map);
    trm_set_line_coeff(map);
    trm_set_mesh(map);
    trm_set_seen(map);

    trm_free_mesh(map);
    trm_free_obj_hiding(map);

    trm_set_reading_star(map);
}
