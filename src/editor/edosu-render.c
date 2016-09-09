#include <math.h>

#include "edosu-render.h"
#include "osux/hitobject.h"
#include "osux/hit.h"
#include "osux/error.h"
#include "osux/util.h"

#ifndef SQUARE
#define SQUARE(x) ((x)*(x))
#endif

typedef struct bezier_point_ {
    double x, y;
} bezier_point;

typedef struct edosu_arc_spline_ {
    double x, y; // center coordinate
    double r;    // radius;
    double a1, a2; // delimiter angles;
    gboolean direct;
    // when coordinates origin is up left
    // direct is clockwise and indirect is anticlockwise
} edosu_arc_spline;

typedef struct edosu_line_ {
    double x1, y1, x2, y2;
} edosu_line;


static void stroke_slider_body(cairo_t *cr, edosu_color *cl)
{
    cairo_set_line_width(cr, 80);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_stroke_preserve(cr); // white border
    cairo_set_line_width(cr, 76);
    cairo_set_source_rgba(cr, cl->r, cl->g, cl->b, cl->a);
    cairo_stroke(cr); // color inner
}

static void draw_object_number(osux_hitobject *ho, cairo_t *cr)
{
    cairo_font_extents_t fe;
    cairo_text_extents_t te;

    cairo_set_font_size(cr, 30);
    cairo_font_extents(cr, &fe);
    cairo_set_source_rgb(cr, 1., 1., 1.);

    char number[20];
    sprintf(number, "%d", ho->combo_position);
    cairo_text_extents(cr, number, &te);
    double x = ho->x - te.x_bearing - te.width / 2;
    double y = ho->y - te.y_bearing - te.height / 2;

    cairo_move_to(cr, x, y);
    cairo_show_text(cr, number);
    cairo_new_path(cr);
}

static bezier_point
Bezier_de_Casteljau(uint32_t n, // control point count = n+1
                    double t, // t € [ 0, 1 ]
                    bezier_point W[n+1]) // control points
{
    int s = n+1;
    bezier_point *X = ARRAY_DUP(W, s);
    bezier_point *Y = g_malloc(sizeof*Y * s);
    bezier_point *tmp, p;

    while (s > 1) {
        for (int i = 0; i < s-1; ++i) {
            Y[i].x = (1-t) * X[i].x + t * X[i+1].x;
            Y[i].y = (1-t) * X[i].y + t * X[i+1].y;
        }
        SWAP_POINTER(X, Y, tmp);
        -- s;
    }
    p = X[0];
    g_free(X);
    g_free(Y);
    return p;
}

/*
  build cairo path for a Bezier curve
  this function use directly cairo Bezier functionality when possible
  (i.e when there is 2,3 or 4 control points)
  otherwise the algorithm used to compute the bezier path is the
  'De Casteljau' algorithm
*/
static void build_bezier_path(
    bezier_point *W, // control point array
    int length,      // control point count (size of the array)
    cairo_t *cr      // cairo context
)
{
    cairo_move_to(cr, W[0].x, W[0].y);
    if (length == 2) {
        cairo_line_to(cr, W[1].x, W[1].y);
    } else if (length == 3) {
        cairo_curve_to(cr,
                       W[0].x, W[0].y,
                       W[1].x, W[1].y,
                       W[2].x, W[2].y);
    } else if (length == 4) {
        cairo_curve_to(cr,
                       W[1].x, W[1].y,
                       W[2].x, W[2].y,
                       W[3].x, W[3].y);
    } else if (length > 4) {
        // 3 point per control point section:
        double step = 0.333 / (length-1);
        double t = step;
        while (t < 1) {
            bezier_point p = { 0.0, 0.0 };
            p = Bezier_de_Casteljau(length-1, t, W);
            cairo_line_to(cr, p.x, p.y);
            t += step;
        }
    }
}

/*
  build cairo path for 'B' type slider
  'B' slider are either Bezier curve or multiple Bezier curved joined together
*/
static void build_B_path(osux_hitobject *ho, cairo_t *cr)
{
    unsigned pc = ho->slider.point_count;
    bezier_point *W = g_malloc(sizeof*W * pc);
    bezier_point *X = W;
    int last_pos = 0;
    for (unsigned i = 0; i < pc; ++i) {
        W[i].x = ho->slider.points[i].x;
        W[i].y = ho->slider.points[i].y;
        if (i && W[i].x == W[i-1].x && W[i].y == W[i-1].y) {
            build_bezier_path(X, i-last_pos, cr);
            last_pos = i;
            X = W+i;
        }
    }
    build_bezier_path(X, pc-last_pos, cr);
    g_free(W);
}

/*
  build a cairo path for a straight line
  TODO: maybe this should be turned into a macro
*/
static inline void
build_straight_line_path(cairo_t *cr, edosu_line *l)
{
    cairo_move_to(cr, l->x1, l->y1);
    cairo_line_to(cr, l->x2, l->y2);
}

static void draw_slider_ball(cairo_t *cr, edosu_color *cl, double x, double y)
{
    (void) cl; // TODO: use color for slider ball

    cairo_arc(cr, x, y, 40, 0, 2*M_PI);
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_stroke(cr); // slider ball is red circle
}


static double
compute_path_lenth(cairo_path_t *path)
{
    bool have_prev = false;
    bezier_point current;
    double length = 0.0;
    cairo_path_data_t *data;

    for (int i = 0; i < path->num_data; i += path->data[i].header.length) {
        data = &path->data[i];
        switch (data->header.type) {
        case CAIRO_PATH_MOVE_TO:
            have_prev = true;
            current.x = data[1].point.x; current.y = data[1].point.y;
            break;
        case CAIRO_PATH_LINE_TO:
            if (have_prev) {
                length += sqrt(SQUARE(current.x-data[1].point.x)+
                               SQUARE(current.y-data[1].point.y));
            }
            have_prev = true;
            current.x = data[1].point.x; current.y = data[1].point.y;
            break;
        case CAIRO_PATH_CURVE_TO:
        case CAIRO_PATH_CLOSE_PATH:
            break;
        }
    }
    return length;
}

/*
  compute position of slider ball on a 'B type' slider
  for the given percentage of completion
*/
static bezier_point
build_B_position(osux_hitobject *ho, cairo_path_t *path, double pct)
{
    bool have_prev = false;
    bezier_point current, p = { ho->x, ho->y };
    double length = compute_path_lenth(path);
    double target = pct * length;
    double arrow = 0.0;
    cairo_path_data_t *data;

    for (int i = 0; i < path->num_data; i += path->data[i].header.length) {
        data = &path->data[i];
        switch (data->header.type) {
        case CAIRO_PATH_MOVE_TO:
            have_prev = true;
            current.x = data[1].point.x; current.y = data[1].point.y;
            break;
        case CAIRO_PATH_LINE_TO:
            if (have_prev) {
                double l = sqrt(SQUARE(current.x-data[1].point.x)+
                                SQUARE(current.y-data[1].point.y));
                if (arrow+l >= target) {
                    pct = (target-arrow) / l;
                    p.x = current.x * (1-pct) + data[1].point.x * pct;
                    p.y = current.y * (1-pct) + data[1].point.y * pct;
                    return p;
                }
                arrow += l;
            }
            have_prev = true;
            current.x = data[1].point.x; current.y = data[1].point.y;
            break;
        case CAIRO_PATH_CURVE_TO:
        case CAIRO_PATH_CLOSE_PATH:
            break;
        }
    }
    return p;
}

/*
  compute position of slider ball on a straight line slider
  for the given percentage
*/
static bezier_point
build_straight_line_position(edosu_line *l, double pct)
{
    bezier_point p = { l->x1, l->y1 };
    if (pct < 0) {
        // computing is meaning less if the slider is yet to be triggered
        // output value will be disregarded when pct < 0;
        return p;
    }
    p.x = l->x1 * (1-pct) + l->x2 * pct;
    p.y = l->y1 * (1-pct) + l->y2 * pct;
    return p;
}

static void draw_slider_control_points(osux_hitobject *ho, cairo_t *cr)
{
    unsigned pc = ho->slider.point_count;
    for (unsigned i = 0; i < pc; ++i) {
        cairo_set_source_rgb(cr, .4, .4, .4);
        if (i > 0 && !memcmp(ho->slider.points+i,
                             ho->slider.points+i-1, sizeof(osux_point)))
            cairo_set_source_rgb(cr, 1., 0., 0);
        cairo_rectangle(cr, ho->slider.points[i].x-4,
                        ho->slider.points[i].y-4, 8, 8);
        cairo_fill(cr);
    }
    cairo_move_to(cr, ho->x, ho->y);
    cairo_set_source_rgb(cr, 0., 1., 0.);
    for (unsigned i = 1; i < pc; ++i)
        cairo_line_to(cr, ho->slider.points[i].x,  ho->slider.points[i].y);
    cairo_stroke(cr);
}

static double angle_main_measure(double a)
{
    while (a > M_PI)
        a -= 2*M_PI;
    while (a <= -M_PI)
        a += 2*M_PI;
    return a;
}

static gboolean make_arc_spline(osux_hitobject *ho, edosu_arc_spline *as)
{
    double x1 = ho->slider.points[0].x, y1 = ho->slider.points[0].y;
    double x2 = ho->slider.points[1].x, y2 = ho->slider.points[1].y;
    double x3 = ho->slider.points[2].x, y3 = ho->slider.points[2].y;
    double Xa = 0.5*(x1+x2), Ya=0.5*(y1+y2);
    double Xb = 0.5*(x2+x3), Yb=0.5*(y2+y3);
    double A1 = y2-y1, B1 = x2-x1;
    double A2 = y3-y2, B2 = x3-x2;

    double det = B1*A2-B2*A1;
    if (fabs(det) < 0.000001)
        return FALSE;

    double p = (A2*B1*Xa - (B2*Xb - A2*Ya + A2*Yb)*A1)/det;
    double q = -(A1*B2*Ya + (B2*Xa - B2*Xb - A2*Yb)*B1)/det;
    double r = sqrt(SQUARE(x1-p) + SQUARE(y1-q));
    double a1 = atan((y1-q)/(x1-p));
    double a2 = atan((y3-q)/(x3-p));
    if (x1-p < 0)
        a1 -= M_PI;
    if (x3-p < 0)
        a2 -= M_PI;

    as->x = p;
    as->y = q;
    as->r = r;
    as->a1 = angle_main_measure(a1);
    as->a2 = angle_main_measure(a2);
    as->direct = det > 0;

    return TRUE;
}

static void build_arc_spline_path(edosu_arc_spline *as, cairo_t *cr)
{
    if (as->direct)
        cairo_arc(cr, as->x, as->y, as->r, as->a1, as->a2);
    else
        cairo_arc_negative(cr, as->x, as->y, as->r, as->a1, as->a2);
}

static bezier_point
build_arc_spline_position(edosu_arc_spline *as, double pct)
{
    double a1 = as->a1, a2 = as->a2;
    if (!as->direct) {
        if (a2 - a1 > 0)
            a2 -= 2*M_PI;
    } else {
        if (a2 - a1 < 0)
            a1 -= 2*M_PI;
    }

    bezier_point pos;
    double a3 = a1 + (a2-a1)*pct;
    pos.x = as->x+as->r*cos(a3);
    pos.y = as->y+as->r*sin(a3);
    return pos;
}

static void draw_slider_edges(osux_hitobject *ho, cairo_t *cr, edosu_color *cl)
{
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgba(cr, cl->r, cl->g, cl->b, cl->a);

    int i = ho->slider.point_count-1;
    double x = ho->slider.points[i].x, y = ho->slider.points[i].y;
    cairo_arc(cr, x, y, 40, 0, 2 * M_PI);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_stroke(cr); // white border (end edge)

    // round first point and fill it
    cairo_arc(cr, ho->x, ho->y, 40, 0, 2 * M_PI);

    cairo_set_source_rgba(cr, cl->r, cl->g, cl->b, cl->a);
    cairo_fill_preserve(cr);// fill circle with color (start edge)

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_stroke(cr); //  white border (start edge)
}

static void draw_slider(osux_hitobject *ho, cairo_t *cr,
                        edosu_color *cl, int64_t local_offset)
{
    bezier_point slider_ball_pos;
    unsigned pc = ho->slider.point_count;
    int tick_length = (ho->end_offset-ho->offset);
    double length = (double) tick_length / (double)ho->slider.repeat;
    double pct = (double)-local_offset/length;
    if (ho->slider.repeat > 1) {
        int i = ((int) 100.*pct ) / 100;
        pct = pct - (int) pct;
        if (i % 2 == 1)
            pct = 1. - pct; // reverse !!
    }

    if (ho->slider.type == 'C') {
        draw_slider_control_points(ho, cr);
        osux_warning("Catmull sliders are unsupported\n");
        return;
    }

    if (pc == 2) { // 'L' type slider: straight line
        edosu_line l = {
            ho->x, ho->y,
            ho->slider.points[1].x, ho->slider.points[1].y
        };
        build_straight_line_path(cr, &l);
        slider_ball_pos = build_straight_line_position(&l, pct);
    } else if (pc == 3) { // 'P' type slider: 'circular' line (arc)
        edosu_arc_spline as;
        if (make_arc_spline(ho, &as)) { // an arc can be formed:
            build_arc_spline_path(&as, cr);
            slider_ball_pos = build_arc_spline_position(&as, pct);
        } else { // no arc can be formed (the 3 points are perfectly aligned)
            edosu_line l = {
                ho->x, ho->y,
                ho->slider.points[2].x, ho->slider.points[2].y
            };
            build_straight_line_path(cr, &l);
            slider_ball_pos = build_straight_line_position(&l, pct);
        }
    } else if (pc > 3) {// 'B' type slider: bezier or piecewise bezier
        build_B_path(ho, cr);
        cairo_path_t *path = cairo_copy_path_flat(cr);
        slider_ball_pos = build_B_position(ho, path, pct);
        cairo_path_destroy(path);
    }
    stroke_slider_body(cr, cl);
    draw_slider_edges(ho, cr, cl);
    draw_object_number(ho, cr);

    if (local_offset < 0)
        draw_slider_ball(cr, cl, slider_ball_pos.x, slider_ball_pos.y);
    if (pc >= 3)
        draw_slider_control_points(ho, cr);
}

static void draw_circle(osux_hitobject *ho, cairo_t *cr, edosu_color *cl)
{
    cairo_arc(cr, ho->x, ho->y, 40, 0, 2 * M_PI);
    cairo_set_source_rgba(cr, cl->r, cl->g, cl->b, cl->a);
    cairo_fill_preserve(cr); // color fill
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_stroke(cr); // white border
    draw_object_number(ho, cr);
}

static void draw_spinner(cairo_t *cr)
{
    cairo_arc(cr, 256, 192, 200, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, 0, 0, 1); // blue fill
    cairo_fill_preserve(cr);
    cairo_set_line_width(cr, 10);
    cairo_set_source_rgb(cr, 0, 0, 0); // black border
    cairo_stroke(cr);
    cairo_set_line_width(cr, 2);

    cairo_arc(cr, 256, 192, 20, 0, 2*M_PI);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_fill(cr);// gray screw !
}

static void draw_approach_circle(osux_hitobject *ho, cairo_t *cr,
                                 edosu_color *cl, int64_t local_offset)
{
    double approach_time = osux_get_approach_time(9.0, 0);
    double pct = local_offset / approach_time;
    cairo_arc(cr, ho->x, ho->y, 40+40*pct, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, cl->r, cl->g, cl->b);
    cairo_stroke(cr);
}

void edosu_draw_object(osux_hitobject *ho, cairo_t *cr,
                       int64_t position, edosu_color *cl)
{
    int64_t local_offset = ho->offset - position;
    int64_t local_end_offset = ho->end_offset - position;

    if (local_end_offset <= 0)
        return;
    if (HIT_OBJECT_IS_CIRCLE(ho))
        draw_circle(ho, cr, cl);
    else if (HIT_OBJECT_IS_SLIDER(ho))
        draw_slider(ho, cr, cl, local_offset);
    else if (HIT_OBJECT_IS_SPINNER(ho) && local_offset < 0)
        draw_spinner(cr);

    if (local_offset < 0 || HIT_OBJECT_IS_SPINNER(ho))
        return;
    draw_approach_circle(ho, cr, cl, local_offset);
}
