#ifndef OSUX_EDITOR_APP_WIN_H
#define OSUX_EDITOR_APP_WIN_H

#include <gtk/gtk.h>
#include "app.h"

G_BEGIN_DECLS

#define OSUX_EDITOR_APP_WINDOW_TYPE (osux_editor_app_window_get_type ())

G_DECLARE_FINAL_TYPE(OsuxEditorAppWindow, osux_editor_app_window,
                     OSUX_EDITOR, APP_WINDOW, GtkApplicationWindow);

OsuxEditorAppWindow *osux_editor_app_window_new(OsuxEditorApp *app);
void osux_editor_app_window_open(OsuxEditorAppWindow *win, GFile *file);

G_END_DECLS


#endif //OSUX_EDITOR_APP_WIN_H
