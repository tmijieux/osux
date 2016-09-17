#include <math.h>

#include "vosu-render.h"
#include "osux/hitobject.h"
#include "osux/hitsound.h"

static void
draw_roll(vosu_renderer const *r,
          osux_hitobject *ho, int64_t local_offset)
{
    cairo_t *cr = r->cr;
    double x1, x2;
    x1 = 100.0 + (double) local_offset / 3.;
    x2 = 100.0 + (double) (ho->end_offset - r->position) / 3.;
    cairo_move_to(cr, x1, 192);
    cairo_line_to(cr, x2, 192);
    int size = ho->hitsound.sample & SAMPLE_TAIKO_BIG ? 40 : 30;
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_set_line_width(cr, 2*size);
    cairo_stroke(cr);
}

static void
draw_shaker(vosu_renderer const *r,
            osux_hitobject *ho, int64_t local_offset)
{
    (void) ho;
    cairo_t *cr = r->cr;
    if (local_offset >= 0)
        return;

    cairo_arc(cr, 256, 192, 100, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_fill_preserve(cr);
    cairo_set_line_width(cr, 10);
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_stroke(cr);
    cairo_set_line_width(cr, 2);

    cairo_arc(cr, 256, 192, 20, 0, 2*M_PI);
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_fill(cr);
}

static void
draw_circle(vosu_renderer const *r,
            osux_hitobject *ho, int64_t local_offset)
{
    cairo_t *cr = r->cr;
    int size = ho->hitsound.sample & SAMPLE_TAIKO_BIG ? 40 : 30;
    double x;
    x = 100.0 + (double) local_offset / 3;

    cairo_arc(cr, x, 192, size, 0, 2*M_PI);
    if (ho->hitsound.sample & SAMPLE_TAIKO_KAT)
        cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    else
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_fill(cr);
}

void
vosu_draw_object_taiko(vosu_renderer const *r, osux_hitobject *ho)
{
    int64_t local_offset;
    local_offset = ho->offset - r->position;

    if (HIT_OBJECT_IS_CIRCLE(ho))
        draw_circle(r, ho, local_offset);
    else if (HIT_OBJECT_IS_SPINNER(ho))
        draw_shaker(r, ho, local_offset);
    else if (HIT_OBJECT_IS_SLIDER(ho))
        draw_roll(r, ho, local_offset);
}

void
vosu_draw_playfield_taiko(vosu_renderer const *r)
{
    cairo_t *cr = r->cr;

    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_arc(cr, 35, 192, 30, M_PI/2., 3*M_PI/2.);
    cairo_close_path(cr);
    cairo_fill(cr);

    cairo_arc(cr, 40, 192, 30, -M_PI/2., M_PI/2.);
    cairo_close_path(cr);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_arc(cr, 35, 192, 35, M_PI/2., 3*M_PI/2.);
    cairo_stroke(cr);

    cairo_arc(cr, 40, 192, 35, -M_PI/2., M_PI/2.);
    cairo_stroke(cr);
}
