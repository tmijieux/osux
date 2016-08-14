#include "popover_search_bar.h"

struct _OsuxEditorPopoverSearchbar
{
    GtkPopover parent;
};

struct _OsuxEditorPopoverSearchbarClass
{
    GtkPopoverClass parent_class;
};

G_DEFINE_TYPE(OsuxEditorPopoverSearchbar,
              osux_editor_popover_searchbar,
              GTK_TYPE_POPOVER);

static void
osux_editor_popover_searchbar_init(OsuxEditorPopoverSearchbar *win)
{
    gtk_widget_init_template(GTK_WIDGET(win));
}

static void
osux_editor_popover_searchbar_class_init(OsuxEditorPopoverSearchbarClass *class)
{
    gtk_widget_class_set_template_from_resource(
        GTK_WIDGET_CLASS(class), "/org/osux/editor/ui/OsuxEditorPopover.ui");
}

OsuxEditorPopoverSearchbar *
osux_editor_popover_searchbar_new(GtkWidget *parent)
{
    return g_object_new(OSUX_EDITOR_POPOVER_SEARCHBAR_TYPE,
                        "relative_to", parent, NULL);
}
