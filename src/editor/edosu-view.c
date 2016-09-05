#include <gtk/gtk.h>
#include <stdint.h>
#include <math.h>

#include "edosu-view.h"
#include "bezier.h"
#include "osux/hitobject.h"
#include "osux/hit.h"
#include "osux/error.h"

#define SQUARE(x) ((x)*(x))

G_DEFINE_TYPE(EdosuView, edosu_view, GTK_TYPE_BOX);


typedef struct edosu_color_ {
    double r, g, b, a;
} edosu_color;


static void edosu_color_get(edosu_color *cl, osux_hitobject *ho)
{
    if (ho->combo_color == NULL) {
        cl->r = 0.6;
        cl->g = 0.6;
        cl->b = 0.6;
        cl->a = 1.0;
        return;
    }
    cl->r = (double) ho->combo_color->r / 255.;
    cl->g = (double) ho->combo_color->g / 255.;
    cl->b = (double) ho->combo_color->b / 255.;
    cl->a = (double) ho->combo_color->a / 255.;
}

static gint
get_object_end_offset(gconstpointer _a, gconstpointer _b, gpointer user_data)
{
    (void) user_data;
    return ((osux_hitobject*)_a)->end_offset - ((osux_hitobject*)_b)->end_offset;
}

static void update_view_position(GtkAdjustment *adj, EdosuView *self)
{
    self->position = gtk_adjustment_get_value(adj);
    gtk_widget_queue_draw(GTK_WIDGET(self->drawing_area));
}

static bool
object_is_approach_time_or_slider(osux_hitobject *ho, int64_t position)
{
    int approach_time;
    int64_t local_offset = ho->offset - position;
    int64_t local_end_offset = ho->end_offset - position;
    approach_time = osux_get_approach_time(9.0, 0);
    return local_offset < approach_time && local_end_offset >= 0;
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

void build_bezier_path(bezier_point *W, int length, cairo_t *cr)
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
        printf("building bezier curve with %d points\n", length);
        double step = 0.1;
        double t = 0.1;

        while (t < 1) {
            bezier_point p = { 0.0, 0.0 };
            p = Bezier(length-1, t, W);
            /* printf("bezier point: x=%g, y=%g\n", p.x, p.y); */
            cairo_line_to(cr, p.x, p.y);
            t += step;
        }
    }
}

static void draw_slider_bezier(osux_hitobject *ho, cairo_t *cr, edosu_color *cl)
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
    
    cairo_set_line_width(cr, 80);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_stroke_preserve(cr); // white border
    cairo_set_line_width(cr, 76);
    cairo_set_source_rgba(cr, cl->r, cl->g, cl->b, cl->a);
    cairo_stroke(cr); // color inner
}

static void draw_slider_control_points(osux_hitobject *ho, cairo_t *cr)
{
    unsigned pc = ho->slider.point_count;
    for (unsigned i = 0; i < pc; ++i) {
        cairo_set_source_rgba(cr, 1., 0., 0., 0.7);

        /* printf("control point: x=%d, y=%d\n", */
        /*        ho->slider.points[i].x, */
        /*        ho->slider.points[i].y); */
        if (i > 0 && !memcmp(ho->slider.points+i,
                             ho->slider.points+i-1, sizeof(osux_point)))
            cairo_set_source_rgb(cr, 0., 0., 0);
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

static void draw_circular_path(osux_hitobject *ho, cairo_t *cr)
{

    double x1 = ho->slider.points[0].x, y1 = ho->slider.points[0].y;
    double x2 = ho->slider.points[1].x, y2 = ho->slider.points[1].y;
    double x3 = ho->slider.points[2].x, y3 = ho->slider.points[2].y;
    double Xa = 0.5*(x1+x2), Ya=0.5*(y1+y2);
    double Xb = 0.5*(x2+x3), Yb=0.5*(y2+y3);
    double A1 = y2-y1, B1 = x2-x1;
    double A2 = y3-y2, B2 = x3-x2;

    double det = B1*A2-B2*A1;
    if (fabs(det) < 0.000001) {
        cairo_move_to(cr, x1, y1);
        cairo_line_to(cr, x3, y3);
        return;
    }
    double p = (A2*B1*Xa - (B2*Xb - A2*Ya + A2*Yb)*A1)/det;
    double q = -(A1*B2*Ya + (B2*Xa - B2*Xb - A2*Yb)*B1)/det;
    double r = sqrt(SQUARE(x1-p) + SQUARE(y1-q));
    double a1 = atan((y1-q)/(x1-p));
    double a2 = atan((y3-q)/(x3-p));
    if (x1-p < 0)
        a1 -= M_PI;
    if (x3-p < 0)
        a2 -= M_PI;
     if (det > 0)
        cairo_arc(cr, p, q, r, a1, a2);
    else
        cairo_arc_negative(cr, p, q, r, a1, a2);
}

static void draw_slider(osux_hitobject *ho, cairo_t *cr, edosu_color *cl,
                        int64_t local_offset)
{
    double line_width = cairo_get_line_width(cr);
    
    cairo_set_source_rgba(cr, cl->r, cl->g, cl->b, cl->a);
    cairo_set_line_width(cr, 80);
    
    unsigned pc = ho->slider.point_count;
    if (pc == 2) { // straight line
        cairo_move_to(cr, ho->x, ho->y);
        cairo_line_to(cr, ho->slider.points[1].x,ho->slider.points[1].y);
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_stroke_preserve(cr); // white border
        cairo_set_line_width(cr, 76);
        cairo_set_source_rgba(cr, cl->r, cl->g, cl->b, cl->a);
        cairo_stroke(cr); // color inner
        if (local_offset < 0) {
            double pct = (double)(-local_offset)
                / (double)(ho->end_offset-ho->offset);
            double x = ho->x * (1-pct) + ho->slider.points[1].x * pct;
            double y = ho->y * (1-pct) + ho->slider.points[1].y * pct;
            cairo_set_line_width(cr, 2);
            cairo_arc(cr, x, y, 40, 0, 2*M_PI);
            cairo_set_source_rgb(cr, 1, 0, 0);
            cairo_stroke(cr);
        }
    } else if (pc == 3) { // 'circular' spline
        draw_circular_path(ho, cr);
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_stroke_preserve(cr); // white border
        cairo_set_line_width(cr, 76);
        cairo_set_source_rgba(cr, cl->r, cl->g, cl->b, cl->a);
        cairo_stroke(cr); // color inner
    } else if (pc > 3) // bezier or piecewise bezier
        draw_slider_bezier(ho, cr, cl);
    
    // restore stroke properties:
    cairo_set_line_width(cr, line_width);
        
    int i = ho->slider.point_count-1;
    // round last point
    cairo_arc(cr, ho->slider.points[i].x, ho->slider.points[i].y,
              40, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, 1, 1, 1); 
    cairo_stroke(cr); // white border

    // round first point and fill it
    cairo_arc(cr, ho->x, ho->y, 40, 0, 2 * M_PI);
    cairo_set_source_rgba(cr, cl->r, cl->g, cl->b, cl->a);
    cairo_fill_preserve(cr);// fill circle color
    cairo_set_source_rgb(cr, 1, 1, 1); 
    cairo_stroke(cr); //  white border

    if (pc>=3)
        draw_slider_control_points(ho, cr);
    draw_object_number(ho, cr);
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

static void draw_object(osux_hitobject *ho, cairo_t *cr,
                        int64_t position, edosu_color *cl)
{
    double approach_time = osux_get_approach_time(9.0, 0);
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
    // draw approach circle
    
    double pct = local_offset / approach_time;
    cairo_arc(cr, ho->x, ho->y, 40+40*pct, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, cl->r, cl->g, cl->b);
    cairo_stroke(cr);
}

static float _w = 667., _h = 499.;

static void draw(GtkDrawingArea *drawing_area, cairo_t *cr, EdosuView *self)
{
    /* cairo_set_source_rgb(cr, 0.2, 0.0, 0.2); */
    /* cairo_rectangle(cr, 0, 0, _w, _h); */
    /* cairo_fill(cr); */
    if (self->hitobjects == NULL)
        return;

    cairo_scale(cr, _w/667., _h/499.);
    cairo_translate(cr, 77, 57);
    cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    
    osux_hitobject key, *obj;
    GSequenceIter *iter = NULL;
    key.offset = self->position-1;
    key.end_offset = self->position-1;
    iter = g_sequence_search(self->hitobjects, &key, get_object_end_offset, NULL);
    //iter = g_sequence_iter_next(iter);

    GList *list = NULL;
    while (!g_sequence_iter_is_end(iter) &&
           (obj = g_sequence_get(iter)) &&
           object_is_approach_time_or_slider(obj, self->position))
    {
        list = g_list_prepend(list, (gpointer) obj);
        iter = g_sequence_iter_next(iter);
    }

    GList *itr = list;
    while (itr != NULL) {
        edosu_color cl;
        obj = (osux_hitobject*) itr->data;
        edosu_color_get(&cl, obj);
        draw_object(obj, cr, self->position, &cl);
        itr = itr->next;
    }
    g_list_free(list);
}

static gboolean animate_play(GtkWidget *widget,
                             GdkFrameClock *frame_clock,
                             gpointer data)
{
    EdosuView *self = data;
    gint64 frame_time = gdk_frame_clock_get_frame_time(frame_clock);

    if (self->first_frame_time == 0)
    {
        self->start_position = self->position;
        self->first_frame_time = frame_time;
        return G_SOURCE_CONTINUE;
    }
    // frame time is in micro seconds
    gint64 d = (frame_time - self->first_frame_time) / 1000; 
    gtk_adjustment_set_value(self->time_adjust, d + self->start_position);
    return G_SOURCE_CONTINUE;
}

static void
pause_clicked(EdosuView *self)
{
    if (self->playing)
    {
        printf("pause play\n");
        gtk_widget_remove_tick_callback(
            GTK_WIDGET(self->drawing_area), self->tick_id);
        self->playing = FALSE;
        self->tick_id = 0;
        self->first_frame_time = 0;
    }
}

static void
play_clicked(EdosuView *self)
{
    if (!self->playing)
    {
        printf("start play\n");
        self->playing = TRUE;
        self->first_frame_time = 0;
        self->tick_id = gtk_widget_add_tick_callback(
            GTK_WIDGET(self->drawing_area), animate_play, self, NULL);
    }
 }

static void resize(GtkDrawingArea *drawing_area, GdkRectangle*rec)
{
    _w = rec->width;
    _h = rec->height;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
}

static gboolean range_clicked(EdosuView *self)
{
    self->first_frame_time = 0;
    return FALSE;
}

static void
edosu_view_init(EdosuView *self)
{
    gtk_widget_init_template(GTK_WIDGET(self));
    g_signal_connect(self->time_adjust, "value-changed",
                     G_CALLBACK(update_view_position), self);
    self->time_max = gtk_adjustment_get_upper(self->time_adjust);
    g_signal_connect(self->drawing_area, "draw", G_CALLBACK(draw), self);
    g_signal_connect(self->drawing_area, "size-allocate",
                     G_CALLBACK(resize), NULL);
    g_signal_connect_swapped(
        self->play_button, "clicked", G_CALLBACK(play_clicked), self);
    g_signal_connect_swapped(
        self->pause_button, "clicked", G_CALLBACK(pause_clicked), self);
    self->playing = FALSE;
    g_signal_connect_swapped(
        self->time_range, "change-value", G_CALLBACK(range_clicked), self);
}

static void
edosu_view_class_init(EdosuViewClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        widget_class, "/org/osux/edosu/ui/EdosuView.glade");
    gtk_widget_class_bind_template_child(widget_class, EdosuView, time_adjust);
    gtk_widget_class_bind_template_child(widget_class, EdosuView, time_range);
    gtk_widget_class_bind_template_child(widget_class, EdosuView, drawing_area);
    gtk_widget_class_bind_template_child(widget_class, EdosuView, play_button);
    gtk_widget_class_bind_template_child(widget_class, EdosuView, pause_button);
}

EdosuView *edosu_view_new(void)
{
    return EDOSU_VIEW(g_object_new(EDOSU_TYPE_VIEW, NULL));
}

void edosu_view_set_max_time(EdosuView *self, uint64_t max_time, double page_range)
{
    self->time_max = max_time;
    gtk_adjustment_set_upper(self->time_adjust, (gdouble) max_time);
    gtk_range_set_fill_level(self->time_range, (gdouble) max_time);
    gtk_adjustment_set_page_increment(self->time_adjust, page_range);
}

void edosu_view_set_hit_objects(EdosuView *self, GSequence *hitobjects)
{
    //edosu_gl_set_hit_objects(view->gl_area, hitobjects);
    self->hitobjects = hitobjects;
}
