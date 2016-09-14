#include <sys/time.h>
#include <gtk/gtk.h>
#include <stdint.h>

#include "vosu-view.h"
#include "vosu-render.h"
#include "osux/hitobject.h"
#include "osux/hit.h"
#include "osux/error.h"

G_DEFINE_TYPE(VosuView, vosu_view, GTK_TYPE_BOX);

static gint
get_object_end_offset(gconstpointer _a, gconstpointer _b, gpointer user_data)
{
    (void) user_data;
    return ((osux_hitobject*)_a)->end_offset - ((osux_hitobject*)_b)->end_offset;
}

static gint
get_cursor_offset(gconstpointer _a, gconstpointer _b, gpointer user_data)
{
    (void) user_data;
    return ((osux_replay_data*)_a)->time_offset -
        ((osux_replay_data*)_b)->time_offset;
}

osux_replay_data *get_cursor(VosuView *self)
{
    osux_replay_data key;
    GSequenceIter *iter = NULL;
    key.time_offset = self->position-1;
    iter = g_sequence_search(self->cursor_data, &key,
                             get_cursor_offset, NULL);
    return g_sequence_get(iter);
}

#define OBJECT_CAN_DRAW(ho, current, approach_time)      \
    (((ho)->offset - (current)) < (approach_time) &&    \
     ((ho)->end_offset - (current)) >= 0)

static GList *get_draw_objects(VosuView *self)
{
    osux_hitobject key, *obj;
    GSequenceIter *iter = NULL;
    key.offset = self->position-1;
    key.end_offset = self->position-1;
    iter = g_sequence_search(self->hitobjects, &key, get_object_end_offset, NULL);
    //iter = g_sequence_iter_next(iter);

    GList *list = NULL;
    while (!g_sequence_iter_is_end(iter) &&
           (obj = g_sequence_get(iter)) &&
           OBJECT_CAN_DRAW(obj, self->position, self->renderer.approach_time))
    {
        list = g_list_prepend(list, (gpointer) obj);
        iter = g_sequence_iter_next(iter);
    }
    return list;
}

static void
draw_objects(VosuView *self, GList *objects)
{
    osux_hitobject *obj;
    GList *itr = objects;
    self->renderer.position = self->position;

    while (itr != NULL) {
        obj = (osux_hitobject*) itr->data;
        vosu_draw_object(&self->renderer, obj);
        itr = itr->next;
    }
}

static float _w = 614.4, _h = 460.8;
static void
draw(GtkDrawingArea *drawing_area, cairo_t *cr, VosuView *self)
{
    (void) drawing_area;

    int64_t draw_length = 0.0;
    for (int i = 0; i < 100; ++i)
        draw_length += self->draw_length[i];
    draw_length /= 100;

    int64_t pos = vosu_player_get_time(self->player) / GST_MSECOND;
    self->position = pos + draw_length / 1000;
    uint64_t minutes = self->position/60000;
    uint64_t seconds = self->position/1000 - minutes*60;

    if (seconds - self->seconds) {
        printf("\rTime = %02ld:%02ld | Draw length average: %5ld µs         ",
               minutes, seconds, draw_length);
        self->seconds = seconds;
        fflush(stdout);
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    if (self->hitobjects == NULL)
        return;
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_scale(cr, _w/614.4, _h/460.8); // screen size
    cairo_rectangle(cr, 0, 0, 614.4, 460.8);
    cairo_translate(cr, 51.2, 38.4); // margin
    cairo_rectangle(cr, 0, 0, 512, 384);
    cairo_stroke(cr);

    GList *list = get_draw_objects(self);
    self->renderer.cr = cr;
    draw_objects(self, list);
    g_list_free(list);

    if (self->cursor_data != NULL) {
        osux_replay_data *cursor;
        cursor = get_cursor(self);
        vosu_draw_cursor(&self->renderer, cursor);
    }
    gettimeofday(&end, NULL);
    guint64 length = (end.tv_sec-start.tv_sec)*1000000
        +(end.tv_usec-start.tv_usec);
    ++ self->draw_length_id;
    self->draw_length_id %= 100;
    self->draw_length[self->draw_length_id] = length;
}

static gboolean
animate_play(GtkWidget *widget,
             GdkFrameClock *clock, gpointer self_ptr)
{
    (void) widget;
    (void) clock;
    VosuView *self = VOSU_VIEW(self_ptr);
    gint64 stream_time = vosu_player_get_time(self->player);
    gint64 pos = stream_time / GST_MSECOND;
    gtk_adjustment_set_value(self->time_adjust, pos);
    return G_SOURCE_CONTINUE;
}

static void
pause_clicked(VosuView *self)
{
    if (self->playing)
    {
        gtk_widget_remove_tick_callback(
            GTK_WIDGET(self->drawing_area), self->tick_id);
        self->playing = FALSE;
        self->tick_id = 0;
        vosu_player_pause(self->player);
    }
}

static void
play_clicked(VosuView *self)
{
    if (!self->playing)
    {
        self->playing = TRUE;
        self->tick_id = gtk_widget_add_tick_callback(
            GTK_WIDGET(self->drawing_area), animate_play, self, NULL);
        vosu_player_play(self->player);
    }
}

static void range_value_changed_cb(GtkAdjustment *adj, VosuView *self)
{
    gtk_widget_queue_draw(GTK_WIDGET(self->drawing_area));
    self->position = gtk_adjustment_get_value(adj);
    if (self->position >= self->time_max-1)
        pause_clicked(self);
}

static void resize(GtkDrawingArea *drawing_area, GdkRectangle*rec)
{
    _w = rec->width;
    _h = rec->height;
    gtk_widget_queue_draw(GTK_WIDGET(drawing_area));
}

static gboolean
range_clicked(VosuView *self, GtkScrollType *scroll_type, gdouble value)
{
    int64_t offset = value;
    if (scroll_type != GTK_SCROLL_NONE)
        vosu_player_seek(self->player, offset);
    return FALSE;
}

static void
vosu_view_init(VosuView *self)
{
    gtk_widget_init_template(GTK_WIDGET(self));
    g_signal_connect(self->time_adjust, "value-changed",
                     G_CALLBACK(range_value_changed_cb), self);
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
    self->player = vosu_player_new();

    memset(self->draw_length, 0, sizeof self->draw_length);
    self->draw_length_id = 0;
}

static void
vosu_view_finalize(GObject *obj)
{
    (void) obj;
    G_OBJECT_CLASS(vosu_view_parent_class)->finalize(obj);
}

static void
vosu_view_dispose(GObject *obj)
{
    VosuView *view = VOSU_VIEW(obj);
    g_clear_object(&view->player);
    G_OBJECT_CLASS(vosu_view_parent_class)->dispose(obj);
}

static void
vosu_view_class_init(VosuViewClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        widget_class, "/org/osux/vosu/ui/VosuView.glade");
    gtk_widget_class_bind_template_child(widget_class, VosuView, time_adjust);
    gtk_widget_class_bind_template_child(widget_class, VosuView, time_range);
    gtk_widget_class_bind_template_child(widget_class, VosuView, drawing_area);
    gtk_widget_class_bind_template_child(widget_class, VosuView, play_button);
    gtk_widget_class_bind_template_child(widget_class, VosuView, pause_button);

    G_OBJECT_CLASS(klass)->dispose = &vosu_view_dispose;
    G_OBJECT_CLASS(klass)->finalize = &vosu_view_finalize;
}

VosuView *vosu_view_new(void)
{
    return VOSU_VIEW(g_object_new(VOSU_TYPE_VIEW, NULL));
}

void
vosu_view_set_properties(VosuView *self,
                         uint64_t max_time,
                         double page_range,
                         GSequence *hitobjects,
                         double approach_rate,
                         double circle_size,
                         int mods, gchar const *music_file)
{
    self->hitobjects = hitobjects;
    self->renderer.approach_time = osux_get_approach_time(approach_rate, mods);
    self->renderer.circle_size = 16 + 4 * circle_size;

    self->time_max = max_time;
    gtk_adjustment_set_upper(self->time_adjust, (gdouble) max_time);
    gtk_range_set_fill_level(self->time_range, (gdouble) max_time);
    gtk_adjustment_set_page_increment(self->time_adjust, page_range);
    vosu_player_set_file(self->player, music_file);
}
