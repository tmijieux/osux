#include <gtk/gtk.h>
#include <stdint.h>

#include "edosu-view.h"

G_DEFINE_TYPE(EdosuView, edosu_view, GTK_TYPE_BOX);

static void
edosu_view_init(EdosuView *label)
{
    gtk_widget_init_template(GTK_WIDGET(label));
}

static void
edosu_view_class_init(EdosuViewClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        widget_class, "/org/osux/edosu/ui/EdosuView.glade");
}

EdosuView*edosu_view_new(void)
{
    return EDOSU_VIEW(g_object_new(EDOSU_TYPE_VIEW, NULL));
}
