#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdint.h>
#include <glib/gi18n.h>

#include "osux.h"
#include "app.h"
#include "app_win.h"
#include "beatmap.h"

struct _OsuxEditorApp
{
    GtkApplication parent;
    OsuxEditorWindow *window;

    gboolean activated;
    OsuxEditorBeatmap **beatmaps;
    uint32_t beatmap_count;
    uint32_t beatmap_buf_size;

    uint32_t current_beatmap;
};

G_DEFINE_TYPE(OsuxEditorApp, osux_editor_app, GTK_TYPE_APPLICATION);

static void osux_editor_app_init(OsuxEditorApp *app)
{
    ALLOC_ARRAY(app->beatmaps, app->beatmap_buf_size, 10);
}

static void osux_editor_app_dispose(GObject *obj)
{
    OsuxEditorApp *app = OSUX_EDITOR_APP( obj );

    for (unsigned i = 0; i < app->beatmap_count; ++i)
        g_clear_object(&app->beatmaps[i]);

    G_OBJECT_CLASS(osux_editor_app_parent_class)->dispose(obj);
}

static void osux_editor_app_finalize(GObject *obj)
{
    OsuxEditorApp *app = OSUX_EDITOR_APP( obj );
    g_free(app->beatmaps);
    app->beatmaps = NULL;
    app->beatmap_buf_size = 0;
    app->beatmap_count = 0;
    G_OBJECT_CLASS (osux_editor_app_parent_class)->finalize(obj);
}

static void
osux_editor_app_open_one(OsuxEditorApp *app, GFile *file)
{
    gchar *path = g_file_get_path(file);
    if (path != NULL) {
        OsuxEditorBeatmap *beatmap;
        beatmap = osux_editor_beatmap_new(path);
        if (beatmap != NULL) {
            /* TODO check that beatmap is not already present */
            HANDLE_ARRAY_SIZE(app->beatmaps,
                              app->beatmap_count,
                              app->beatmap_buf_size);
            app->beatmaps[app->beatmap_count++] = beatmap;
        }
        g_free(path);
    }
}

static void osux_editor_app_save_current_beatmap(
    OsuxEditorApp *app, char const *filename)
{
    (void) app;
    (void) filename;
}

/* ---- actions */

static void
quit_action(GSimpleAction *action,
            GVariant      *parameter,
            gpointer       app)
{
    (void) action;
    (void) parameter;
    g_application_quit(G_APPLICATION(app));
}

static void
open_action(GSimpleAction *action,
            GVariant      *parameter,
            gpointer       app_ptr)
{
    (void) action;
    (void) parameter;
    gint res;
    GtkWidget *dialog;
    GtkFileChooserAction fc_action = GTK_FILE_CHOOSER_ACTION_OPEN;
    OsuxEditorApp *app = OSUX_EDITOR_APP( app_ptr );
    OsuxEditorWindow *win = app->window;

    dialog = gtk_file_chooser_dialog_new (
        _("Open File"), GTK_WINDOW(win), fc_action, _("_Cancel"),
        GTK_RESPONSE_CANCEL, _("_Open"), GTK_RESPONSE_ACCEPT, NULL);

    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
        filename = gtk_file_chooser_get_filename(chooser);

        GFile *file = g_file_new_for_path(filename);
        osux_editor_app_open_one(app, file);
        if (app->beatmap_count) {
            OsuxEditorBeatmap *beatmap = app->beatmaps[app->beatmap_count-1];
            osux_editor_window_add_beatmap_tab(win, beatmap);
        }
        g_object_unref(file);
        g_free(filename);
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
    gint res;
    GtkWindow *win = GTK_WINDOW(OSUX_EDITOR_APP(app)->window);

    dialog = gtk_file_chooser_dialog_new(
        _("Save beatmap"), win, GTK_FILE_CHOOSER_ACTION_SAVE, _("_Cancel"),
        GTK_RESPONSE_CANCEL, _("_Save"), GTK_RESPONSE_ACCEPT, NULL);

    res = gtk_dialog_run(GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        osux_editor_app_save_current_beatmap(OSUX_EDITOR_APP(app), filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static GActionEntry app_entries[] = {
    { "quit", &quit_action, NULL, NULL, NULL, {0}},
    { "open", &open_action, NULL, NULL, NULL, {0}},
    { "new", &open_action, NULL, NULL, NULL, {0}},
    { "save", &save_action, NULL, NULL, NULL, {0}},
    { "save_as", &save_action, NULL, NULL, NULL, {0}},
};

/* no more actions */
static void
osux_editor_app_startup(GApplication *app)
{
    G_APPLICATION_CLASS (osux_editor_app_parent_class)->startup(app);
    g_action_map_add_action_entries(
        G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);
}

static void
osux_editor_app_activate(GApplication *gapp)
{
    OsuxEditorApp *app = OSUX_EDITOR_APP( gapp );
    OsuxEditorWindow *win = app->window;
    
    if (!app->activated) {
        win = osux_editor_window_new(app);
        app->window = win;
        if (app->beatmap_count > 0)
            osux_editor_window_add_beatmap_tab(win, app->beatmaps[0]);
        app->activated = TRUE;
    }
    gtk_window_present(GTK_WINDOW(win));
}

static void
osux_editor_app_open(GApplication *gapp,
                     GFile **files,
                     gint n_files,
                     gchar const *hint)
{
    (void) hint;
    int i;
    OsuxEditorApp *app = OSUX_EDITOR_APP( gapp );
    
    for (i = 0; i < n_files; i++)
        osux_editor_app_open_one(app, files[i]);

    OsuxEditorWindow *win = app->window;
    if (win == NULL)
        g_application_activate(gapp);
}

static void
osux_editor_app_class_init(OsuxEditorAppClass *klass)
{
    G_APPLICATION_CLASS(klass)->startup = osux_editor_app_startup;
    G_APPLICATION_CLASS(klass)->activate = osux_editor_app_activate;
    G_APPLICATION_CLASS(klass)->open = osux_editor_app_open;

    G_OBJECT_CLASS(klass)->dispose = osux_editor_app_dispose;
    G_OBJECT_CLASS(klass)->finalize = osux_editor_app_finalize;
}

OsuxEditorApp *
osux_editor_app_new(void)
{
    return g_object_new(OSUX_TYPE_EDITOR_APP,
                        "application-id", "org.osux.editor",
                        "flags", G_APPLICATION_HANDLES_OPEN, NULL);
}

OsuxEditorBeatmap *
osux_editor_app_get_beatmap_by_page(OsuxEditorApp *app, GtkWidget *page)
{
    for (unsigned i = 0; i < app->beatmap_count; ++i) {
        if (app->beatmaps[i]->main_view == page)
            return app->beatmaps[i];
    }
    return NULL;
}
