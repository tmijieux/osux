#include "popsearch.h"

struct _OsuxEditorPopsearch
{
    GtkPopover parent;
};

G_DEFINE_TYPE(OsuxEditorPopsearch, osux_editor_popsearch, GTK_TYPE_POPOVER);

static void
osux_editor_popsearch_init(OsuxEditorPopsearch *win)
{
    gtk_widget_init_template(GTK_WIDGET(win));
}

static void
osux_editor_popsearch_class_init(OsuxEditorPopsearchClass *class)
{
    gtk_widget_class_set_template_from_resource(
        GTK_WIDGET_CLASS(class), "/org/osux/editor/ui/OsuxEditorPopover.ui");
}

OsuxEditorPopsearch *
osux_editor_popsearch_new(GtkWidget *parent)
{
    return g_object_new(OSUX_TYPE_EDITOR_POPSEARCH, "relative_to", parent, NULL);
}
