#include <math.h>

#include "vosu-render.h"
#include "osux/hitobject.h"
#include "osux/hit.h"
#include "osux/error.h"
#include "osux/util.h"

#ifndef SQUARE
#define SQUARE(x) ((x)*(x))
#endif

#define NORM(x, y) sqrt(SQUARE(x)+SQUARE(y))
#define DEGREE_TO_RADIAN(x) ((x) * M_PI / 180.)

typedef struct bezier_point_ {
    double x, y;
} bezier_point;

typedef struct vosu_arc_spline_ {
    double x, y; // center coordinate
    double r;    // radius;
    double a1, a2; // delimiter angles;
    gboolean direct;
    // when coordinates origin is up left
    // direct is clockwise and indirect is anticlockwise
} vosu_arc_spline;

typedef struct vosu_line_ {
    double x1, y1, x2, y2;
} vosu_line;

static void renderer_color_get(vosu_renderer *r, osux_hitobject *ho)
{
    if (ho->combo_color == NULL) {
        r->r = 0.6;
        r->g = 0.6;
        r->b = 0.6;
        r->a = 1.0;
        return;
    }
    r->r = (double) ho->combo_color->r / 255.;
    r->g = (double) ho->combo_color->g / 255.;
    r->b = (double) ho->combo_color->b / 255.;
    r->a = (double) ho->combo_color->a / 255.;
}

static void stroke_slider_body(vosu_renderer const *r)
{
    cairo_t *cr = r->cr;

    cairo_set_line_width(cr, 2*r->circle_size);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_stroke_preserve(cr); // white border
    cairo_set_line_width(cr, 1.85*r->circle_size);
    cairo_set_source_rgba(cr, r->r, r->g, r->b, r->a);
    cairo_stroke(cr); // color inner
}

static void
draw_object_number(vosu_renderer const *r, osux_hitobject *ho)
{
    cairo_t *cr = r->cr;
    cairo_font_extents_t fe;
    cairo_text_extents_t te;

    cairo_set_font_size(cr, 0.75 * r->circle_size);
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
build_straight_line_path(cairo_t *cr, vosu_line *l)
{
    cairo_move_to(cr, l->x1, l->y1);
    cairo_line_to(cr, l->x2, l->y2);
}

static void
draw_slider_ball(vosu_renderer const *r, double x, double y)
{
    cairo_t *cr = r->cr;

    cairo_arc(cr, x, y, r->circle_size, 0, 2*M_PI);
    //cairo_set_line_width(cr, 0.05 * r->circle_size);
    cairo_set_source_rgb(cr, 1-r->r, 1-r->g, 1-r->b);
    cairo_fill(cr);
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
build_straight_line_position(vosu_line *l, double pct)
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
    cairo_set_line_width(cr, 2);
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

static gboolean make_arc_spline(osux_hitobject *ho, vosu_arc_spline *as)
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

static void build_arc_spline_path(vosu_arc_spline *as, cairo_t *cr)
{
    if (as->direct)
        cairo_arc(cr, as->x, as->y, as->r, as->a1, as->a2);
    else
        cairo_arc_negative(cr, as->x, as->y, as->r, as->a1, as->a2);
}

static bezier_point
build_arc_spline_position(vosu_arc_spline *as, double pct)
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

static void draw_circle(vosu_renderer const *r, int x, int y)
{
    cairo_t *cr = r->cr;

    cairo_arc(cr, x, y, r->circle_size, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_fill(cr); // white color fill

    cairo_arc(cr, x, y, 0.925 * r->circle_size, 0, 2 * M_PI);
    cairo_set_source_rgba(cr, r->r, r->g, r->b, r->a);
    cairo_fill(cr); // inner color fill
}

static void
vec2_rotate(double *vx, double *vy, double angle)
{
    double rx, ry;
    double cn = cos(angle);
    double sn = sin(angle);
    rx = *vx * cn - *vy * sn;
    ry = *vx * sn + *vy * cn;
    *vx = rx;
    *vy = ry;
}

static void
draw_slider_reverse_arrow(vosu_renderer const *r, double size_bonus,
                          int x, int y, int tx, int ty)
{
    cairo_t *cr = r->cr;
    double vx, vy, wx, wy;
    vx = tx - x; vy = ty - y;
    double n = NORM(vx, vy);
    vx /= n; vy /= n;
    //vec2_rotate(&vx, &vy, angle);
    wx = vy; wy = -vx;

    double as = (1.2+size_bonus) * r->circle_size;

    cairo_move_to(cr, x-0.35*as*vx, y-0.35*as*vy);
    cairo_rel_line_to(cr, 0.2*as*wx, 0.2*as*wy);
    cairo_rel_line_to(cr, 0.4*as*vx, 0.4*as*vy);
    cairo_rel_line_to(cr, 0.1*as*wx, 0.1*as*wy);

    cairo_rel_line_to(cr,
                      -0.3*as*wx  + 0.2*as*vx,
                      -0.3*as*wy  + 0.2*as*vy);
    cairo_rel_line_to(cr,
                      -0.3*as*wx  + -0.2*as*vx,
                      -0.3*as*wy  + -0.2*as*vy);

    cairo_rel_line_to(cr, 0.1*as*wx, 0.1*as*wy);
    cairo_rel_line_to(cr, -0.4*as*vx, -0.4*as*vy);
    cairo_rel_line_to(cr, 0.2*as*wx, 0.2*as*wy);
    cairo_close_path(cr);

    cairo_set_line_width(cr, 0.05 * r->circle_size);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_fill(cr);
}

typedef osux_point ivec2;

static void
draw_slider_edge(vosu_renderer const *r, bool draw_arrow,
                 double size_bonus, ivec2 *center, ivec2 *eye)
{
    draw_circle(r, center->x, center->y);
    if (draw_arrow)
        draw_slider_reverse_arrow(r, size_bonus,
                                  center->x, center->y,
                                  eye->x, eye->y);
}

static void
get_edges_point(osux_hitobject *ho, bool reverse,
                ivec2 *src, ivec2 *srcd, ivec2 *dst, ivec2 *dstd)
{
    int last = ho->slider.point_count-1;
    if (!reverse) {
        *src = ho->slider.points[0];
        *srcd = ho->slider.points[1];
        *dst = ho->slider.points[last];
        *dstd = ho->slider.points[last-1];
    } else {
        *src = ho->slider.points[last];
        *srcd = ho->slider.points[last-1];
        *dst = ho->slider.points[0];
        *dstd = ho->slider.points[1];
    }
}

static void
draw_circle_object(vosu_renderer const *r, osux_hitobject *ho)
{
    draw_circle(r, ho->x, ho->y);
    draw_object_number(r, ho);
}

static void
draw_slider_edges(vosu_renderer const *r,
                  osux_hitobject *ho, int64_t local_offset)
{
    ivec2 src, srcd, dst, dstd;
    int64_t total_length, length;
    int repeat = ho->slider.repeat;

    total_length = ho->end_offset-ho->offset;
    length = total_length / (repeat);
    int pass = (-local_offset/(int)length);
    pass = MAX(0, MIN(pass, repeat-1));
    bool reverse = pass%2;

    get_edges_point(ho, reverse, &src, &srcd, &dst, &dstd);

    double beatlength = ho->timingpoint->millisecond_per_beat;
    double size_bonus = 0.15*(1.+sin(-local_offset*(M_PI/beatlength)));

    if (local_offset <= 0 && pass != repeat-1)
        draw_slider_edge(r, pass < repeat-2, size_bonus, &src, &srcd);
    draw_slider_edge(r, pass < repeat-1, size_bonus, &dst, &dstd);

    if (local_offset > 0)
        draw_circle_object(r, ho);
}

static void
draw_slider(vosu_renderer const *r,
            osux_hitobject *ho, int64_t local_offset)
{
    cairo_t *cr = r->cr;
    bezier_point slider_ball_pos;
    unsigned pc = ho->slider.point_count;
    int total_length = (ho->end_offset-ho->offset);
    double length = (double) total_length / (double)ho->slider.repeat;
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
        vosu_line l = {
            ho->x, ho->y,
            ho->slider.points[1].x, ho->slider.points[1].y
        };
        build_straight_line_path(cr, &l);
        slider_ball_pos = build_straight_line_position(&l, pct);
    } else if (pc == 3) { // 'P' type slider: 'circular' line (arc)
        vosu_arc_spline as;
        if (make_arc_spline(ho, &as)) { // an arc can be formed:
            build_arc_spline_path(&as, cr);
            slider_ball_pos = build_arc_spline_position(&as, pct);
        } else { // no arc can be formed (the 3 points are perfectly aligned)
            vosu_line l = {
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
    stroke_slider_body(r);
    draw_slider_edges(r, ho, local_offset);

    if (local_offset < 0)
        draw_slider_ball(r, slider_ball_pos.x, slider_ball_pos.y);
    /*
    if (pc >= 3)
        draw_slider_control_points(ho, cr);
    */
}

static void draw_spinner(cairo_t *cr)
{
    cairo_arc(cr, 256, 192, 150, 0, 2 * M_PI);
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

static void
draw_approach_circle(vosu_renderer const *r, osux_hitobject *ho,
                     int64_t local_offset)
{
    cairo_t *cr = r->cr;
    double pct = (double) local_offset / r->approach_time;
    double size = r->circle_size * ( 1.0 + 1.2 * pct );

    cairo_set_line_width(cr, 0.05 * size);
    cairo_arc(cr, ho->x, ho->y, size, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, r->r, r->g, r->b);
    cairo_stroke(cr);
}



void vosu_draw_object(vosu_renderer *r, osux_hitobject *ho)
{
    int64_t local_offset = ho->offset - r->position;
    int64_t local_end_offset = ho->end_offset - r->position;
    renderer_color_get(r, ho);

    if (local_end_offset <= 0)
        return;
    if (HIT_OBJECT_IS_CIRCLE(ho))
        draw_circle_object(r, ho);
    else if (HIT_OBJECT_IS_SLIDER(ho))
        draw_slider(r, ho, local_offset);
    else if (HIT_OBJECT_IS_SPINNER(ho) && local_offset < 0)
        draw_spinner(r->cr);

    if (local_offset < 0 || HIT_OBJECT_IS_SPINNER(ho))
        return;
    draw_approach_circle(r, ho, local_offset);
}

void
vosu_draw_cursor(vosu_renderer const *r, osux_replay_data *cursor)
{
    cairo_t *cr = r->cr;
    double size = 0.5 * r->circle_size;

    cairo_arc(cr, cursor->x, cursor->y, size, 0, 2 * M_PI);
    cairo_set_source_rgba(cr, 255, 0, 0, 180);
    cairo_fill(cr);
}
