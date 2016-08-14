#ifndef OSUX_EDITOR_POPOVER_SEARCH_BAR_H
#define OSUX_EDITOR_POPOVER_SEARCH_BAR_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define OSUX_EDITOR_POPOVER_SEARCHBAR_TYPE \
    (osux_editor_popover_searchbar_get_type ())

G_DECLARE_FINAL_TYPE(OsuxEditorPopoverSearchbar, osux_editor_popover_searchbar,
                     OSUX_EDITOR, POPOVER_SEARCHBAR, GtkPopover);

OsuxEditorPopoverSearchbar *osux_editor_popover_searchbar_new(GtkWidget *parent);

G_END_DECLS


#endif //OSUX_EDITOR_POPOVER_SEARCH_BAR_H
