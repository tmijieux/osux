#ifndef EDOSU_WINDOW_H
#define EDOSU_WINDOW_H

#include <gtk/gtk.h>
#include "edosu-app.h"
#include "edosu-beatmap.h"

G_BEGIN_DECLS

#define EDOSU_TYPE_WINDOW (edosu_window_get_type())

G_DECLARE_FINAL_TYPE(EdosuWindow, edosu_window,
                     EDOSU, WINDOW, GtkApplicationWindow);

struct _EdosuWindow {
    GtkApplicationWindow parent;
    EdosuApplication *app;

    GtkNotebook *view_notebook;
    GtkNotebook *inspector_notebook;
    GtkNotebook *palette_notebook;
    GtkNotebook *properties_notebook;

    guint beatmap_count;
};

EdosuWindow *edosu_window_new(EdosuApplication *app);
void edosu_window_add_beatmap(EdosuWindow *win, EdosuBeatmap *beatmap);
void edosu_window_remove_beatmap(EdosuWindow *win, EdosuBeatmap *beatmap);
void edosu_window_focus_beatmap(EdosuWindow *win, EdosuBeatmap *beatmap);

G_END_DECLS


#endif //EDOSU_WINDOW_H
