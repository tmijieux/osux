#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "app.h"
#include "app_win.h"

struct _OsuxEditorApp
{
    GtkApplication parent;
    OsuxEditorWindow *window;
};

G_DEFINE_TYPE(OsuxEditorApp, osux_editor_app, GTK_TYPE_APPLICATION);

static void osux_editor_app_init(OsuxEditorApp *app)
{
    (void) app;
}

static void
quit_action(GSimpleAction *action,
            GVariant      *parameter,
            gpointer       app)
{
    (void) action;
    (void) parameter;
    g_application_quit(G_APPLICATION (app));
}

static void
open_action(GSimpleAction *action,
            GVariant      *parameter,
            gpointer       app)
{
    (void) action;
    (void) parameter;

    GtkWidget *dialog;
    GtkFileChooserAction fc_action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;
    GtkWindow *win = GTK_WINDOW(OSUX_EDITOR_APP(app)->window);

    dialog = gtk_file_chooser_dialog_new (
        _("Open File"), win, fc_action, _("_Cancel"),
        GTK_RESPONSE_CANCEL, _("_Open"), GTK_RESPONSE_ACCEPT, NULL);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename(chooser);

        GFile *file = g_file_new_for_path(filename);
        osux_editor_window_open(OSUX_EDITOR_WINDOW(win), file);
        g_object_unref(file);
        g_free (filename);
    }

    gtk_widget_destroy (dialog);
}

static void
save_action(GSimpleAction *action,
            GVariant      *parameter,
            gpointer       app)
{
    (void) action;
    (void) parameter;

    GtkWidget *dialog;
    GtkFileChooserAction fc_action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;
    GtkWindow *win = GTK_WINDOW(OSUX_EDITOR_APP(app)->window);

    dialog = gtk_file_chooser_dialog_new (
        _("Save File"), win, fc_action, _("_Cancel"),
        GTK_RESPONSE_CANCEL, _("_Save"), GTK_RESPONSE_ACCEPT, NULL);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename(chooser);

        GFile *file = g_file_new_for_path(filename);
        osux_editor_window_open(OSUX_EDITOR_WINDOW(win), file);
        g_object_unref(file);
        g_free (filename);
    }

    gtk_widget_destroy (dialog);
}

static GActionEntry app_entries[] = {
    { "quit", &quit_action, NULL, NULL, NULL, {0} },
    { "open", &open_action, NULL, NULL, NULL, {0}},
    { "new", &open_action, NULL, NULL, NULL, {0} },
    { "save", &save_action, NULL, NULL, NULL, {0} },
    { "save_as", &save_action, NULL, NULL, NULL, {0}},
};

static void
osux_editor_app_startup(GApplication *app)
{
    G_APPLICATION_CLASS (osux_editor_app_parent_class)->startup(app);
    g_action_map_add_action_entries(
        G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);
}

static void
osux_editor_app_activate(GApplication *app)
{
    OsuxEditorWindow *win;
    if ( OSUX_EDITOR_APP(app)->window == NULL ) {
        win = osux_editor_window_new(OSUX_EDITOR_APP(app));
        OSUX_EDITOR_APP(app)->window = win;
    } else
        win = OSUX_EDITOR_APP(app)->window;
    gtk_window_present(GTK_WINDOW(win));
}

static void
osux_editor_app_open(GApplication  *app,
                     GFile        **files,
                     gint           n_files,
                     const gchar   *hint)
{
    (void) hint;
    OsuxEditorWindow *win = OSUX_EDITOR_APP(app)->window;
    if (win == NULL) {
        win = osux_editor_window_new(OSUX_EDITOR_APP(app));
        OSUX_EDITOR_APP(app)->window = win;
    }
    int i;

    for (i = 0; i < n_files; i++)
        osux_editor_window_open(win, files[i]);
    gtk_window_present(GTK_WINDOW(win));
}

static void
osux_editor_app_class_init(OsuxEditorAppClass *class)
{
    G_APPLICATION_CLASS(class)->startup = osux_editor_app_startup;
    G_APPLICATION_CLASS(class)->activate = osux_editor_app_activate;
    G_APPLICATION_CLASS(class)->open = osux_editor_app_open;
}

OsuxEditorApp *
osux_editor_app_new(void)
{
    return g_object_new(OSUX_TYPE_EDITOR_APP,
                        "application-id", "org.osux.editor",
                        "flags", G_APPLICATION_HANDLES_OPEN,
                        NULL);
}
