#include <gtk/gtk.h>
#include <stdint.h>

#include "osux.h"
#include "vosu-app.h"
#include "vosu-win.h"
#include "vosu-beatmap.h"

G_DEFINE_TYPE(VosuWindow, vosu_window, GTK_TYPE_APPLICATION_WINDOW);

static void
vosu_window_class_init(VosuWindowClass *klass)
{
    GtkWidgetClass *k = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        k, "/org/osux/vosu/ui/VosuWindow.glade");
    gtk_widget_class_bind_template_child(k, VosuWindow, view_notebook);
    gtk_widget_class_bind_template_child(k, VosuWindow, replay_file_chooser);
    gtk_widget_class_bind_template_child(k, VosuWindow, close_replay_button);
}

static void
replay_file_set_cb(GtkFileChooser *chooser, VosuWindow *win)
{
    gchar *filename = gtk_file_chooser_get_filename(chooser);
    if (!vosu_application_open_replay(win->app, filename))
        gtk_file_chooser_unselect_all(chooser);
    else
        gtk_widget_show(GTK_WIDGET(win->close_replay_button));
    g_free(filename);
}

static void
close_replay_clicked_cb(VosuWindow *win)
{
    vosu_application_close_replay(win->app);
    gtk_widget_hide(GTK_WIDGET(win->close_replay_button));
    gtk_file_chooser_unselect_all(win->replay_file_chooser);
}

static void
vosu_window_init(VosuWindow *win)
{
    gtk_widget_init_template(GTK_WIDGET(win));
    g_signal_connect(win->replay_file_chooser, "file-set",
                     G_CALLBACK(replay_file_set_cb), win);
    g_signal_connect_swapped(win->close_replay_button, "clicked",
                     G_CALLBACK(close_replay_clicked_cb), win);
}

VosuWindow *
vosu_window_new(VosuApplication *app)
{
    VosuWindow *w;
    w = VOSU_WINDOW(g_object_new(VOSU_TYPE_WINDOW, "application", app, NULL));
    w->app = app;
    gtk_file_chooser_add_filter(w->replay_file_chooser, app->osr_file_filter);
    gtk_file_chooser_add_filter(w->replay_file_chooser, app->all_file_filter);
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

VosuView *vosu_window_get_view(VosuWindow *win)
{
    int n = gtk_notebook_get_n_pages(win->view_notebook);
    if (n != 1)
        return NULL;
    return VOSU_VIEW(gtk_notebook_get_nth_page(win->view_notebook, 0));
}
