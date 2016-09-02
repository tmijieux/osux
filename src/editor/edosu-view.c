#include <gtk/gtk.h>
#include <stdint.h>

#include "edosu-view.h"

G_DEFINE_TYPE(EdosuView, edosu_view, GTK_TYPE_BOX);

static void update_view_position(GtkAdjustment *adj, EdosuGL *gl_area)
{
    double value = gtk_adjustment_get_value(adj);
    edosu_gl_set_position(gl_area, value);
}

static void
edosu_view_init(EdosuView *self)
{
    gtk_widget_init_template(GTK_WIDGET(self));
    g_signal_connect(self->time_adjust, "value-changed",
                     G_CALLBACK(update_view_position), self->gl_area);
    self->time_max = gtk_adjustment_get_upper(self->time_adjust);
}

static void
edosu_view_class_init(EdosuViewClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        widget_class, "/org/osux/edosu/ui/EdosuView.glade");
    gtk_widget_class_bind_template_child(widget_class, EdosuView, time_adjust);
    gtk_widget_class_bind_template_child(widget_class, EdosuView, time_range);
    gtk_widget_class_bind_template_child(widget_class, EdosuView, gl_area);
}

EdosuView *edosu_view_new(void)
{
    return EDOSU_VIEW(g_object_new(EDOSU_TYPE_VIEW, NULL));
}

void edosu_view_set_max_time(EdosuView *self, uint64_t max_time)
{
    self->time_max = max_time;
    gtk_adjustment_set_upper(self->time_adjust, (gdouble) max_time);
    gtk_range_set_fill_level(self->time_range, (gdouble) max_time);
}


void edosu_view_set_hit_objects(EdosuView *view, GSequence *hitobjects)
{
    edosu_gl_set_hit_objects(view->gl_area, hitobjects);
}
