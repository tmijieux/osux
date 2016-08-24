#include <gtk/gtk.h>
#include <stdint.h>

#include "edosu-properties.h"

G_DEFINE_TYPE(EdosuProperties, edosu_properties, GTK_TYPE_NOTEBOOK);

static void
edosu_properties_init(EdosuProperties *label)
{
    gtk_widget_init_template(GTK_WIDGET(label));
}

static void
edosu_properties_class_init(EdosuPropertiesClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        widget_class, "/org/osux/edosu/ui/EdosuProperties.glade");
}

EdosuProperties*edosu_properties_new(void)
{
    return EDOSU_PROPERTIES(g_object_new(EDOSU_TYPE_PROPERTIES, NULL));
}
