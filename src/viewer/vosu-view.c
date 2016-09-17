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
    iter = vosu_sequence_search(self->cursor_data, &key,
                                get_cursor_offset, NULL);
    if (g_sequence_iter_is_end(iter))
        return NULL;
    return g_sequence_get(iter);
}

#define OBJECT_CAN_DRAW(ho, current, approach_time)      \
    (((ho)->offset - (current)) < (approach_time) &&    \
     ((ho)->end_offset - (current)) >= 0)

static void
draw_objects(VosuView *self)
{
    osux_hitobject key, *obj;
    GSequenceIter *iter = NULL;
    VosuSequence *objects = vosu_sequence_ref(self->hitobjects_mod);

    key.offset = self->position-1;
    key.end_offset = self->position-1;
    iter = vosu_sequence_search(
        objects, &key, get_object_end_offset, NULL);

    GList *no_spinner = NULL, *spinner = NULL;
    while (!g_sequence_iter_is_end(iter) &&
           (obj = g_sequence_get(iter)) &&
           OBJECT_CAN_DRAW(obj, self->position,
                           self->renderer.approach_time))
    {
        if (HIT_OBJECT_IS_SPINNER(obj))
            spinner = g_list_prepend(spinner, (gpointer) obj);
        else
            no_spinner = g_list_prepend(no_spinner, (gpointer) obj);
        iter = g_sequence_iter_next(iter);
    }
    // always draw spinner first (-->  behind other objects)
    spinner = g_list_concat(spinner, no_spinner);

    GList *itr = spinner;
    self->renderer.position = self->position;

    while (itr != NULL) {
        obj = (osux_hitobject*) itr->data;
        vosu_draw_object(&self->renderer, obj);
        itr = itr->next;
    }
    vosu_sequence_unref(objects);
}

static void
draw_handle_time_end(VosuView *self)
{
    int64_t minutes = self->position/60000;
    int64_t seconds = self->position/1000 - minutes*60;
    int64_t diff = self->position - self->info_display_old_time;

    if (diff < 0 || diff > 100) {
        printf("\rTime = %02ld:%02ld     ", minutes, seconds);
        self->info_display_old_time = self->position;
        fflush(stdout);
    }
}

static float _w = 614.4, _h = 460.8;
static void
draw_init_cairo(VosuView *self, cairo_t *cr)
{
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_scale(cr, _w/614.4, _h/460.8); // screen size
    self->renderer.cr = cr;
}

static void
draw_playfield_limit(VosuView *self)
{
    cairo_t *cr = self->renderer.cr;

    cairo_rectangle(cr, 0, 0, 614.4, 460.8);
    cairo_translate(cr, 51.2, 38.4); // margin
    cairo_rectangle(cr, 0, 0, 512, 384);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_stroke(cr);
}


static void
draw(GtkDrawingArea *drawing_area, cairo_t *cr, VosuView *self)
{
    (void) drawing_area;
    int64_t pos;
    if (self->playing)
        pos = vosu_player_get_time(self->player) / GST_MSECOND;
    else
        pos = gtk_adjustment_get_value(self->time_adjust);
    self->position = pos;

    if (self->hitobjects == NULL)
        return;

    draw_init_cairo(self, cr);
    draw_playfield_limit(self);
    vosu_draw_playfield(&self->renderer);
    draw_objects(self);

    if (self->cursor_data != NULL) {
        osux_replay_data *cursor;
        cursor = get_cursor(self);
        if (cursor != NULL)
            vosu_draw_cursor(&self->renderer, cursor);
    }
    draw_handle_time_end(self);
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
    if (self->position >= self->time_max)
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
}

static void
vosu_view_dispose(GObject *obj)
{
    VosuView *view = VOSU_VIEW(obj);
    g_clear_object(&view->player);
    g_clear_object(&view->hitobjects);
    g_clear_object(&view->hitobjects_mod);
    G_OBJECT_CLASS(vosu_view_parent_class)->dispose(obj);
}

static void
vosu_view_finalize(GObject *obj)
{
    VosuView *view = VOSU_VIEW(obj);
    (void) view;
    G_OBJECT_CLASS(vosu_view_parent_class)->finalize(obj);
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
vosu_view_set_beatmap_properties(
    VosuView *self,
    uint64_t max_time,    double page_range,
    int64_t game_mode,    VosuSequence *hitobjects,
    double approach_rate, double circle_size,
    gchar const *music_file)
{
    self->hitobjects = vosu_sequence_ref(hitobjects);
    self->hitobjects_mod = vosu_sequence_ref(hitobjects);

    self->renderer.game_mode = game_mode;
    self->renderer.approach_rate = approach_rate;
    self->renderer.approach_time = osux_get_approach_time(approach_rate, 0);
    self->renderer.base_circle_size = circle_size;
    self->renderer.circle_size = osux_get_circle_size(circle_size, 0);

    self->time_max = max_time;
    gtk_adjustment_set_upper(self->time_adjust, (gdouble) max_time);
    gtk_range_set_fill_level(self->time_range, (gdouble) max_time);
    gtk_adjustment_set_page_increment(self->time_adjust, page_range);
    vosu_player_set_file(self->player, music_file);
}

static VosuSequence*
copy_hitobject_seq(VosuView *view, int mods)
{
    VosuSequence *hitobjects = view->hitobjects, *cpy;
    GSequenceIter *it; osux_hitobject *nhos;

    cpy = vosu_sequence_new((GDestroyNotify)osux_hitobject_free);
    it = vosu_sequence_get_begin_iter(hitobjects);

    int length = vosu_sequence_get_length(hitobjects);
    nhos = g_malloc0(sizeof nhos[0] * length);

    int i = 0;
    while (!g_sequence_iter_is_end(it)) {
        osux_hitobject *ho, *nho;
        nho = &nhos[i++];
        ho = g_sequence_get(it);
        osux_hitobject_copy(ho, nho);
        osux_hitobject_apply_mods(nho, mods);
        vosu_sequence_append(cpy, nho);
        it = g_sequence_iter_next(it);
    }
    vosu_sequence_add_additional_data(
        cpy, nhos, (GDestroyNotify)g_free);
    return cpy;
}

void
vosu_view_set_replay_properties(
    VosuView *view, VosuSequence *cursor_data, int mods)
{
    view->cursor_data = cursor_data;

    double at;
    at = osux_get_approach_time(view->renderer.approach_rate, mods);
    view->renderer.approach_time = at;

    int cs;
    cs = osux_get_circle_size(view->renderer.base_circle_size, mods);
    view->renderer.circle_size = cs;

    VosuSequence *old = view->hitobjects_mod;
    if (mods != 0)
        view->hitobjects_mod = copy_hitobject_seq(view, mods);
    else
        view->hitobjects_mod = vosu_sequence_ref(view->hitobjects);
    vosu_sequence_unref(old);
}
