#include <gtk/gtk.h>
#include <stdint.h>

#include "app.h"
#include "app_win.h"
#include "beatmap.h"
#include "popsearch.h"

struct _OsuxEditorWindow
{
    GtkApplicationWindow parent;

    GtkWidget *popover;
    GtkWidget *new_circle_button;

    OsuxEditorBeatmap **beatmaps;
    uint32_t beatmap_count;
};

G_DEFINE_TYPE(OsuxEditorWindow,
              osux_editor_window,
              GTK_TYPE_APPLICATION_WINDOW);

static void
osux_editor_window_init(OsuxEditorWindow *win)
{
    gtk_widget_init_template(GTK_WIDGET(win));
    win->popover = GTK_WIDGET(
        osux_editor_popsearch_new(win->new_circle_button) );

    win->beatmaps = NULL;
    win->beatmap_count = 0;
}

static void
osux_editor_window_class_init(OsuxEditorWindowClass *klass)
{
    GtkWidgetClass *wklass = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        wklass, "/org/osux/editor/ui/OsuxEditorWindow.ui");
    gtk_widget_class_bind_template_child(
        wklass, OsuxEditorWindow, new_circle_button);
}

OsuxEditorWindow *
osux_editor_window_new(OsuxEditorApp *app)
{
    return g_object_new(OSUX_TYPE_EDITOR_WINDOW, "application", app, NULL);
}

void osux_editor_window_open(OsuxEditorWindow *win, GFile *file)
{
    gchar *path = g_file_get_path(file);
    if (path != NULL) {
        OsuxEditorBeatmap *beatmap;
        beatmap = osux_editor_beatmap_new(path);
        if (beatmap != NULL) {
            /* TODO check that beatmap is not already present */
            (void) win;

            g_object_unref(beatmap);
        }
        g_free(path);
    }
}

gboolean on_main_window_key_press(
    GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    (void) user_data;

    GtkWidget *popover = OSUX_EDITOR_WINDOW(widget)->popover;
    switch (event->keyval) {
    case GDK_KEY_space:
        if (gtk_widget_is_visible(popover))
            gtk_widget_hide(popover);
        else
            gtk_widget_show_all(popover);
        return TRUE;
        break;
    default:
        break;
    };
    return FALSE;
}
