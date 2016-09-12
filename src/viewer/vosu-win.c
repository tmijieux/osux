#include <gtk/gtk.h>
#include <stdint.h>

#include "osux.h"
#include "vosu-app.h"
#include "vosu-win.h"
#include "vosu-beatmap.h"

G_DEFINE_TYPE(VosuWindow, vosu_window, GTK_TYPE_APPLICATION_WINDOW);

static gboolean
on_notebook_page_switch_cb(VosuWindow *win, VosuView *view)
{
    VosuApplication *app = win->app;
    VosuBeatmap *beatmap;

    beatmap = vosu_application_get_beatmap_by_view(app, view);
    if (beatmap == NULL)
        return TRUE;
    gint n;
    n = gtk_notebook_page_num(win->view_notebook, GTK_WIDGET(view));

    vosu_application_set_current_beatmap(app, beatmap);
    gtk_window_set_title(GTK_WINDOW(win), beatmap->filename);
    gtk_notebook_set_current_page(win->inspector_notebook, n);
    gtk_notebook_set_current_page(win->palette_notebook, n);
    gtk_notebook_set_current_page(win->properties_notebook, n);

    return FALSE;
}

static gboolean
on_notebook_page_removed_cb(VosuWindow *window,
                            VosuView *view,
                            guint page_num)
{
    (void) view;
    gtk_notebook_remove_page(window->palette_notebook, page_num);
    gtk_notebook_remove_page(window->inspector_notebook, page_num);
    gtk_notebook_remove_page(window->properties_notebook, page_num);
    return FALSE;
}

static void
vosu_window_init(VosuWindow *win)
{
    gtk_widget_init_template(GTK_WIDGET(win));
    g_signal_connect_swapped(win->view_notebook, "switch-page",
                             G_CALLBACK(on_notebook_page_switch_cb), win);
    g_signal_connect_swapped(win->view_notebook, "page-removed",
                             G_CALLBACK(on_notebook_page_removed_cb), win);
}

static void
vosu_window_class_init(VosuWindowClass *klass)
{
    GtkWidgetClass *k = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        k, "/org/osux/vosu/ui/VosuWindow.glade");
    gtk_widget_class_bind_template_child(k, VosuWindow, view_notebook);
    gtk_widget_class_bind_template_child(k, VosuWindow, inspector_notebook);
    gtk_widget_class_bind_template_child(k, VosuWindow, properties_notebook);
    gtk_widget_class_bind_template_child(k, VosuWindow, palette_notebook);
}

VosuWindow *
vosu_window_new(VosuApplication *app)
{
    VosuWindow *w;
    w = VOSU_WINDOW(g_object_new(VOSU_TYPE_WINDOW, "application", app, NULL));
    w->app = app;
    return w;
}

void vosu_window_add_beatmap(VosuWindow *win, VosuBeatmap *beatmap)
{
    gint n_pages;
    GtkWidget *label;

    n_pages = gtk_notebook_get_n_pages(win->view_notebook);

    // tabs update:
    if (n_pages == 1)
        gtk_notebook_set_show_tabs(win->view_notebook, TRUE);

    label = vosu_close_label_new(beatmap->filename, win->app, beatmap);

    gtk_notebook_append_page(win->view_notebook,
                             GTK_WIDGET(beatmap->view), label);
    gtk_notebook_append_page(win->inspector_notebook,
                             GTK_WIDGET(beatmap->inspector), NULL);
    gtk_notebook_append_page(win->properties_notebook,
                             GTK_WIDGET(beatmap->properties), NULL);
    gtk_notebook_append_page(win->palette_notebook,
                             GTK_WIDGET(beatmap->palette), NULL);

    gtk_notebook_set_current_page(win->view_notebook, -1);
    gtk_notebook_set_current_page(win->properties_notebook, -1);
    gtk_notebook_set_current_page(win->palette_notebook, -1);
    gtk_notebook_set_current_page(win->inspector_notebook, -1);

    ++ win->beatmap_count;
}

void vosu_window_remove_beatmap(VosuWindow *win, VosuBeatmap *beatmap)
{
    gint n;
    n = gtk_notebook_page_num(win->view_notebook, GTK_WIDGET(beatmap->view));
    gtk_notebook_remove_page(win->view_notebook, n);
    -- win->beatmap_count;

    if (!win->beatmap_count) {
        gtk_window_set_title(GTK_WINDOW(win), _("osux Editor"));
        vosu_application_set_current_beatmap(win->app, NULL);
    }
    if (win->beatmap_count == 1)
        gtk_notebook_set_show_tabs(win->view_notebook, FALSE);
}

void vosu_window_focus_beatmap(VosuWindow *win, VosuBeatmap *beatmap)
{
    gint n;
    n = gtk_notebook_page_num(win->view_notebook, GTK_WIDGET(beatmap->view));
    gtk_notebook_set_current_page(win->view_notebook, n);
}
