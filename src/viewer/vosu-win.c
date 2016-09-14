#include <gtk/gtk.h>
#include <stdint.h>

#include "osux.h"
#include "vosu-app.h"
#include "vosu-win.h"
#include "vosu-beatmap.h"

G_DEFINE_TYPE(VosuWindow, vosu_window, GTK_TYPE_APPLICATION_WINDOW);

static void
vosu_window_init(VosuWindow *win)
{
    gtk_widget_init_template(GTK_WIDGET(win));
}

static void
vosu_window_class_init(VosuWindowClass *klass)
{
    GtkWidgetClass *k = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        k, "/org/osux/vosu/ui/VosuWindow.glade");
    gtk_widget_class_bind_template_child(k, VosuWindow, view_notebook);
    gtk_widget_class_bind_template_child(k, VosuWindow, replay_file_chooser);
}

static void
replay_file_set_cb(GtkFileChooser *chooser, VosuApplication *app)
{
    gchar *filename = gtk_file_chooser_get_filename(chooser);
    vosu_application_set_replay_file(app, filename);
    g_free(filename);
}

VosuWindow *
vosu_window_new(VosuApplication *app)
{
    VosuWindow *w;
    w = VOSU_WINDOW(g_object_new(VOSU_TYPE_WINDOW, "application", app, NULL));
    w->app = app;
    gtk_file_chooser_add_filter(w->replay_file_chooser, app->osr_file_filter);
    gtk_file_chooser_add_filter(w->replay_file_chooser, app->all_file_filter);
    g_signal_connect(w->replay_file_chooser, "file-set",
                     G_CALLBACK(replay_file_set_cb), app);
    return w;
}

void vosu_window_close_view(VosuWindow *win)
{
    int n = gtk_notebook_get_n_pages(win->view_notebook);
    osux_debug("n = %d\n", n);
    if (n == 1)
        gtk_notebook_remove_page(win->view_notebook, 0);
}

void vosu_window_set_view(VosuWindow *win, VosuView *view)
{
    vosu_window_close_view(win);
    gtk_notebook_append_page(win->view_notebook, GTK_WIDGET(view), NULL);
}
