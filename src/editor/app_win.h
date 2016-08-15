#ifndef OSUX_EDITOR_WINDOW_H
#define OSUX_EDITOR_WINDOW_H

#include <gtk/gtk.h>
#include "app.h"

G_BEGIN_DECLS

#define OSUX_TYPE_EDITOR_WINDOW (osux_editor_window_get_type())

G_DECLARE_FINAL_TYPE(OsuxEditorWindow, osux_editor_window,
                     OSUX, EDITOR_WINDOW, GtkApplicationWindow);

OsuxEditorWindow *osux_editor_window_new(OsuxEditorApp *app);
void osux_editor_window_open(OsuxEditorWindow *win, GFile *file);

G_END_DECLS


#endif //OSUX_EDITOR_WINDOW_H
