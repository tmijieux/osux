#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdint.h>
#include <glib/gi18n.h>

#include "osux.h"
#include "edosu-app.h"
#include "edosu-win.h"
#include "edosu-beatmap.h"

struct _EdosuApplication
{
    GtkApplication parent;
    EdosuWindow *window;

    GList *beatmaps;
    EdosuBeatmap *current_beatmap;
    EdosuBeatmap *last_loaded_beatmap;

    GtkFileFilter *osu_file_filter;
    GtkFileFilter *osb_file_filter;
    GtkFileFilter *all_file_filter;
};

G_DEFINE_TYPE(EdosuApplication, edosu_application, GTK_TYPE_APPLICATION);

static void edosu_application_init(EdosuApplication *app)
{
    GtkBuilder *builder;
    GObject *osu_file_filter, *osb_file_filter, *all_file_filter;

    builder = gtk_builder_new_from_resource(
        "/org/osux/edosu/ui/EdosuFileFilterOsu.glade");
    all_file_filter = gtk_builder_get_object(builder, "All files");
    osu_file_filter = gtk_builder_get_object(builder, "Beatmaps");
    osb_file_filter = gtk_builder_get_object(builder, "Storyboards");
    g_object_ref(all_file_filter);
    g_object_ref(osu_file_filter);
    g_object_ref(osb_file_filter);

    app->all_file_filter =  GTK_FILE_FILTER(all_file_filter);
    app->osu_file_filter = GTK_FILE_FILTER(osu_file_filter);
    app->osb_file_filter = GTK_FILE_FILTER(osb_file_filter);

    g_object_unref(G_OBJECT(builder));
    app->beatmaps = NULL;
}

static void edosu_application_dispose(GObject *obj)
{
    EdosuApplication *app = EDOSU_APPLICATION( obj );
    g_clear_object(&app->osu_file_filter);
    g_clear_object(&app->osb_file_filter);
    g_clear_object(&app->all_file_filter);
    g_clear_object(&app->beatmaps);

    G_OBJECT_CLASS(edosu_application_parent_class)->dispose(obj);
}

static void edosu_application_finalize(GObject *obj)
{
    G_OBJECT_CLASS (edosu_application_parent_class)->finalize(obj);
}

static void add_beatmap(EdosuApplication *app, EdosuBeatmap *beatmap)
{
    if (g_list_find(app->beatmaps, beatmap) != NULL)
        return;
    app->beatmaps = g_list_append(app->beatmaps, beatmap);
    edosu_window_add_beatmap(app->window, beatmap);
}

static void
do_close(EdosuApplication *app, EdosuBeatmap *beatmap)
{
    edosu_window_remove_beatmap(app->window, beatmap);
    app->beatmaps = g_list_remove(app->beatmaps, beatmap);
}

static gboolean open_beatmap(EdosuApplication *app, gchar *path)
{
    EdosuBeatmap *beatmap;
    beatmap = edosu_beatmap_new();
    if (!beatmap) {
        osux_error(_("cannot create beatmap"));
        return FALSE;
    }

    if (!edosu_beatmap_load_from_file(beatmap, path))
        return FALSE;
    add_beatmap(app, beatmap);    
    return TRUE;
}

static void switch_to_beatmap(EdosuApplication *app, EdosuBeatmap *beatmap)
{
    edosu_window_focus_beatmap(app->window, beatmap);
}

gboolean
edosu_application_open_beatmap(EdosuApplication *app, gchar *path)
{
    EdosuBeatmap *beatmap;
    g_return_val_if_fail(path != NULL, FALSE);

    beatmap = edosu_application_get_beatmap_by_path(app, path);
    if (beatmap != NULL) {
        switch_to_beatmap(app, beatmap);
        return TRUE;
    } else {
        return open_beatmap(app, path);
    }
}

void
edosu_application_set_current_beatmap(EdosuApplication *app, EdosuBeatmap *bm)
{
    app->current_beatmap = bm;
}

void edosu_application_close_beatmap(EdosuApplication *app, EdosuBeatmap *beatmap)
{
    /* TODO check if beatmap has modification */

    do_close(app, beatmap);
}

/* ---- actions */

static void
close_action(GSimpleAction *action, GVariant *parameter, gpointer app_ptr)
{
    (void) action;
    (void) parameter;
    
    EdosuApplication *app = EDOSU_APPLICATION(app_ptr);
    edosu_application_close_beatmap(app, app->current_beatmap);
}

static void
quit_action(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    (void) action;
    (void) parameter;
    g_application_quit(G_APPLICATION(app));
}

static GtkWidget *
create_open_beatmap_dialog(EdosuApplication *app, GtkWindow *window)
{
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new(
        _("Open Beatmap"), window, GTK_FILE_CHOOSER_ACTION_OPEN,
        _("_Cancel"), GTK_RESPONSE_CANCEL,
        _("_Open"), GTK_RESPONSE_ACCEPT, NULL);

    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_add_filter(chooser, app->osu_file_filter);
    gtk_file_chooser_add_filter(chooser, app->osb_file_filter);
    gtk_file_chooser_add_filter(chooser, app->all_file_filter);
    return dialog;
}

static void
open_action(GSimpleAction *action, GVariant *parameter, gpointer app_ptr)
{
    (void) action;
    (void) parameter;
    EdosuApplication *app = EDOSU_APPLICATION(app_ptr);
    GtkWidget *dialog = create_open_beatmap_dialog(app, GTK_WINDOW(app->window));

    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        gchar *filename = gtk_file_chooser_get_filename(chooser);
        edosu_application_open_beatmap(app, filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static void
save_action(GSimpleAction *action, GVariant *parameter, gpointer papp)
{
    (void) action;
    EdosuApplication *app = EDOSU_APPLICATION(papp);
    if (app->current_beatmap == NULL)
        return;

    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new(
        _("Save beatmap"), GTK_WINDOW(app->window), GTK_FILE_CHOOSER_ACTION_SAVE,
        _("_Cancel"), GTK_RESPONSE_CANCEL,
        _("_Save"), GTK_RESPONSE_ACCEPT, NULL);

    gint res = gtk_dialog_run(GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
        edosu_beatmap_save(app->current_beatmap, filename);
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
    { "close", &close_action, NULL, NULL, NULL, {0}},
};

/* ---- actions end */

static void
edosu_application_startup(GApplication *gapp)
{
    EdosuApplication *app = EDOSU_APPLICATION( gapp );
    G_APPLICATION_CLASS(edosu_application_parent_class)->startup(gapp);
    g_action_map_add_action_entries(
        G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);
    app->window = edosu_window_new(app);
}

static void
edosu_application_activate(GApplication *gapp)
{
    EdosuApplication *app = EDOSU_APPLICATION( gapp );
    gtk_widget_show(GTK_WIDGET(app->window));
    gtk_window_present(GTK_WINDOW(app->window));
}

static void
edosu_application_open(GApplication *gapp, GFile **files,
                       gint n_files, gchar const *hint)
{
    (void) hint;
    EdosuApplication *app = EDOSU_APPLICATION( gapp );

    for (int i = 0; i < n_files; i++) {
        gchar *path = g_file_get_path(files[i]);
        edosu_application_open_beatmap(app, path);
        g_free(path);
    }
}

static void
edosu_application_class_init(EdosuApplicationClass *klass)
{
    G_APPLICATION_CLASS(klass)->startup = edosu_application_startup;
    G_APPLICATION_CLASS(klass)->open = edosu_application_open;
    G_APPLICATION_CLASS(klass)->activate = edosu_application_activate;

    G_OBJECT_CLASS(klass)->dispose = edosu_application_dispose;
    G_OBJECT_CLASS(klass)->finalize = edosu_application_finalize;
}

EdosuApplication *
edosu_application_new(void)
{
    return g_object_new(EDOSU_TYPE_APPLICATION,
                        "application-id", "org.osux.edosu",
                        "flags", G_APPLICATION_HANDLES_OPEN, NULL);
}

EdosuBeatmap *
edosu_application_get_beatmap_by_view(EdosuApplication *app, EdosuView *view)
{
    GList *l; 
    for (l = app->beatmaps; l; l = l->next) {
        if (((EdosuBeatmap*) l->data)->view == view)
            return l->data;
    }
    return NULL;
}

EdosuBeatmap *
edosu_application_get_beatmap_by_path(EdosuApplication *app, gchar const *path)
{
    GList *l; 
    for (l = app->beatmaps; l; l = l->next) {
        if (!strcmp(((EdosuBeatmap*) l->data)->filepath, path))
            return l->data;
    }
    return NULL;
}
