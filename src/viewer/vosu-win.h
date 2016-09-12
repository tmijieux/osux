#ifndef VOSU_WINDOW_H
#define VOSU_WINDOW_H

#include <gtk/gtk.h>
#include "vosu-app.h"
#include "vosu-beatmap.h"

G_BEGIN_DECLS

#define VOSU_TYPE_WINDOW (vosu_window_get_type())

G_DECLARE_FINAL_TYPE(VosuWindow, vosu_window,
                     VOSU, WINDOW, GtkApplicationWindow);

struct _VosuWindow {
    GtkApplicationWindow parent;
    VosuApplication *app;
    GtkNotebook *view_notebook;
};

VosuWindow *vosu_window_new(VosuApplication *app);
void vosu_window_set_view(VosuWindow *win, VosuView *view);
void vosu_window_close_view(VosuWindow *win);

G_END_DECLS

#endif //VOSU_WINDOW_H
