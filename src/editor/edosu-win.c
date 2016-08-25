#include <gtk/gtk.h>
#include <stdint.h>

#include "osux.h"
#include "edosu-app.h"
#include "edosu-win.h"
#include "edosu-beatmap.h"
#include "edosu-close-lbl.h"

G_DEFINE_TYPE(EdosuWindow, edosu_window, GTK_TYPE_APPLICATION_WINDOW);

static gboolean
on_notebook_page_switch_cb(EdosuWindow *win, EdosuView *view)
{
    EdosuApplication *app = win->app;
    EdosuBeatmap *beatmap;

    beatmap = edosu_application_get_beatmap_by_view(app, view);
    if (beatmap == NULL)
        return TRUE;
    gint n;
    n = gtk_notebook_page_num(win->view_notebook, GTK_WIDGET(view));

    edosu_application_set_current_beatmap(app, beatmap);
    gtk_window_set_title(GTK_WINDOW(win), beatmap->filename);
    gtk_notebook_set_current_page(win->inspector_notebook, n);
    gtk_notebook_set_current_page(win->palette_notebook, n);
    gtk_notebook_set_current_page(win->properties_notebook, n);

    return FALSE;
}

static gboolean
on_notebook_page_removed_cb(EdosuWindow *window,
                            EdosuView *view,
                            guint page_num)
{
    (void) view;
    gtk_notebook_remove_page(window->palette_notebook, page_num);
    gtk_notebook_remove_page(window->inspector_notebook, page_num);
    gtk_notebook_remove_page(window->properties_notebook, page_num);
    return FALSE;
}

static void
edosu_window_init(EdosuWindow *win)
{
    gtk_widget_init_template(GTK_WIDGET(win));
    g_signal_connect_swapped(win->view_notebook, "switch-page",
                             G_CALLBACK(on_notebook_page_switch_cb), win);
    g_signal_connect_swapped(win->view_notebook, "page-removed",
                             G_CALLBACK(on_notebook_page_removed_cb), win);
}

static void
edosu_window_class_init(EdosuWindowClass *klass)
{
    GtkWidgetClass *k = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        k, "/org/osux/edosu/ui/EdosuWindow.glade");
    gtk_widget_class_bind_template_child(k, EdosuWindow, view_notebook);
    gtk_widget_class_bind_template_child(k, EdosuWindow, inspector_notebook);
    gtk_widget_class_bind_template_child(k, EdosuWindow, properties_notebook);
    gtk_widget_class_bind_template_child(k, EdosuWindow, palette_notebook);
}

EdosuWindow *
edosu_window_new(EdosuApplication *app)
{
    EdosuWindow *w;
    w = EDOSU_WINDOW(g_object_new(EDOSU_TYPE_WINDOW, "application", app, NULL));
    w->app = app;
    return w;
}

void edosu_window_add_beatmap(EdosuWindow *win, EdosuBeatmap *beatmap)
{
    gint n_pages;
    GtkWidget *label;

    n_pages = gtk_notebook_get_n_pages(win->view_notebook);

    // tabs update:
    if (n_pages == 1)
        gtk_notebook_set_show_tabs(win->view_notebook, TRUE);

    label = edosu_close_label_new(beatmap->filename, win->app, beatmap);

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

void edosu_window_remove_beatmap(EdosuWindow *win, EdosuBeatmap *beatmap)
{
    gint n;
    n = gtk_notebook_page_num(win->view_notebook, GTK_WIDGET(beatmap->view));
    gtk_notebook_remove_page(win->view_notebook, n);
    -- win->beatmap_count;

    if (!win->beatmap_count) {
        gtk_window_set_title(GTK_WINDOW(win), _("osux Editor"));
        edosu_application_set_current_beatmap(win->app, NULL);
    }
}

void edosu_window_focus_beatmap(EdosuWindow *win, EdosuBeatmap *beatmap)
{
    gint n;

    n = gtk_notebook_page_num(win->view_notebook, GTK_WIDGET(beatmap->view));
    gtk_notebook_set_current_page(win->view_notebook, n);
}
