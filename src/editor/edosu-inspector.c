#include <gtk/gtk.h>
#include <stdint.h>

#include "edosu-inspector.h"

G_DEFINE_TYPE(EdosuInspector, edosu_inspector, GTK_TYPE_BOX);

static void
edosu_inspector_init(EdosuInspector *label)
{
    gtk_widget_init_template(GTK_WIDGET(label));
}

static void
edosu_inspector_class_init(EdosuInspectorClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        widget_class, "/org/osux/edosu/ui/EdosuInspector.glade");
    gtk_widget_class_bind_template_child(widget_class, EdosuInspector, treeview);
}

EdosuInspector*edosu_inspector_new(void)
{
    return EDOSU_INSPECTOR(g_object_new(EDOSU_TYPE_INSPECTOR, NULL));
}

void edosu_inspector_set_model(EdosuInspector *inspector, GtkTreeModel *model)
{
    gtk_tree_view_set_model(inspector->treeview, model);
}
