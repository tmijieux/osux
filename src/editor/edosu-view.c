#include <gtk/gtk.h>
#include <stdint.h>

#include "edosu-view.h"
#include "edosu-render.h"
#include "osux/hitobject.h"
#include "osux/hit.h"
#include "osux/error.h"

G_DEFINE_TYPE(EdosuView, edosu_view, GTK_TYPE_BOX);

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

static float _w = 667., _h = 499.;

static void draw(GtkDrawingArea *drawing_area, cairo_t *cr, EdosuView *self)
{
    (void) drawing_area;

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
        edosu_draw_object(obj, cr, self->position, &cl);
        itr = itr->next;
    }
    g_list_free(list);
}

static gboolean animate_play(GtkWidget *widget,
                             GdkFrameClock *frame_clock,
                             gpointer data)
{
    (void) widget;

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
