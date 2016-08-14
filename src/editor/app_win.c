#include <gtk/gtk.h>

#include "app.h"
#include "app_win.h"
#include "popover_search_bar.h"

struct _OsuxEditorAppWindow
{
    GtkApplicationWindow parent;

    GtkWidget *popover;
    GtkWidget *new_circle_button;
};

G_DEFINE_TYPE(OsuxEditorAppWindow,
              osux_editor_app_window,
              GTK_TYPE_APPLICATION_WINDOW);

static void
osux_editor_app_window_init(OsuxEditorAppWindow *win)
{
    gtk_widget_init_template(GTK_WIDGET(win));
    win->popover = GTK_WIDGET(
        osux_editor_popover_searchbar_new(win->new_circle_button) );
}

static void
osux_editor_app_window_class_init(OsuxEditorAppWindowClass *klass)
{
    GtkWidgetClass *wklass = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        wklass, "/org/osux/editor/ui/OsuxEditorWindow.ui");
    gtk_widget_class_bind_template_child(
        wklass, OsuxEditorAppWindow, new_circle_button);
}

OsuxEditorAppWindow *
osux_editor_app_window_new(OsuxEditorApp *app)
{
    return g_object_new(OSUX_EDITOR_APP_WINDOW_TYPE, "application", app, NULL);
}

void osux_editor_app_window_open(OsuxEditorAppWindow *win, GFile *file)
{
    gchar *path = g_file_get_path(file);
    if (path != NULL) {
        
        g_free(path);
    }
}

gboolean on_main_window_key_press(
    GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    GtkWidget *popover = OSUX_EDITOR_APP_WINDOW(widget)->popover;
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
