#ifndef VOSU_WINDOW_H
#define VOSU_WINDOW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define VOSU_TYPE_WINDOW (vosu_window_get_type())

G_DECLARE_FINAL_TYPE(VosuWindow, vosu_window,
                     VOSU, WINDOW, GtkApplicationWindow);

#include "vosu-app.h"
#include "vosu-beatmap.h"

struct _VosuWindow {
    GtkApplicationWindow parent;
    VosuApplication *app;
    GtkNotebook *view_notebook;
    GtkFileChooser *replay_file_chooser;
    GtkButton *close_replay_button;

    gboolean fullscreen;
};

VosuWindow *vosu_window_new(VosuApplication *app);
void vosu_window_set_view(VosuWindow *win, VosuView *view);
VosuView *vosu_window_get_view(VosuWindow *win);
void vosu_window_set_replay(VosuWindow *win, VosuView *view);
void vosu_window_close_view(VosuWindow *win);

G_END_DECLS

#endif //VOSU_WINDOW_H
