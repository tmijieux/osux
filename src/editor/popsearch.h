#ifndef OSUX_EDITOR_POPSEARCH_H
#define OSUX_EDITOR_POPSEARCH_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define OSUX_TYPE_EDITOR_POPSEARCH \
    (osux_editor_popsearch_get_type ())

G_DECLARE_FINAL_TYPE(OsuxEditorPopsearch, osux_editor_popsearch,
                     OSUX, EDITOR_POPSEARCH, GtkPopover);

OsuxEditorPopsearch *osux_editor_popsearch_new(GtkWidget *parent);

G_END_DECLS


#endif //OSUX_EDITOR_POPSEARCH_H
