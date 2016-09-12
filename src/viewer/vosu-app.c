#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdint.h>
#include <glib/gi18n.h>

#include "osux.h"
#include "vosu-app.h"
#include "vosu-win.h"
#include "vosu-beatmap.h"

struct _VosuApplication
{
    GtkApplication parent;
    VosuWindow *window;

    GList *beatmaps;
    VosuBeatmap *current_beatmap;
    VosuBeatmap *last_loaded_beatmap;

    GtkFileFilter *osu_file_filter;
    GtkFileFilter *osb_file_filter;
    GtkFileFilter *all_file_filter;
};

G_DEFINE_TYPE(VosuApplication, vosu_application, GTK_TYPE_APPLICATION);

static void vosu_application_init(VosuApplication *app)
{
    GtkBuilder *builder;
    GObject *osu_file_filter, *osb_file_filter, *all_file_filter;

    builder = gtk_builder_new_from_resource(
        "/org/osux/vosu/ui/VosuFileFilterOsu.glade");
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

static void vosu_application_dispose(GObject *obj)
{
    VosuApplication *app = VOSU_APPLICATION( obj );
    g_clear_object(&app->osu_file_filter);
    g_clear_object(&app->osb_file_filter);
    g_clear_object(&app->all_file_filter);
    g_clear_object(&app->beatmaps);

    G_OBJECT_CLASS(vosu_application_parent_class)->dispose(obj);
}

static void vosu_application_finalize(GObject *obj)
{
    G_OBJECT_CLASS (vosu_application_parent_class)->finalize(obj);
}

static void add_beatmap(VosuApplication *app, VosuBeatmap *beatmap)
{
    if (g_list_find(app->beatmaps, beatmap) != NULL)
        return;
    app->beatmaps = g_list_append(app->beatmaps, beatmap);
    vosu_window_add_beatmap(app->window, beatmap);
}

static void
do_close(VosuApplication *app, VosuBeatmap *beatmap)
{
    vosu_window_remove_beatmap(app->window, beatmap);
    app->beatmaps = g_list_remove(app->beatmaps, beatmap);
}

static void
vosu_application_error(VosuApplication *app,
                        gchar const *title, gchar const *error)
{
    GtkDialog *dialog;
    dialog = GTK_DIALOG(gtk_message_dialog_new(
        GTK_WINDOW(app->window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK, "%s", error));
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(dialog);
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

static gboolean open_beatmap(VosuApplication *app, gchar *path)
{
    VosuBeatmap *beatmap;
    beatmap = vosu_beatmap_new();
    if (!beatmap) {
        osux_error(_("cannot create beatmap"));
        return FALSE;
    }

    if (!vosu_beatmap_load_from_file(beatmap, path)) {
        vosu_application_error(app, _("Error loading beatmap"), beatmap->errmsg);
        g_object_unref(G_OBJECT(beatmap));
        return FALSE;
    }
    add_beatmap(app, beatmap);
    return TRUE;
}

static void switch_to_beatmap(VosuApplication *app, VosuBeatmap *beatmap)
{
    vosu_window_focus_beatmap(app->window, beatmap);
}

gboolean
vosu_application_open_beatmap(VosuApplication *app, gchar *path)
{
    VosuBeatmap *beatmap;
    g_return_val_if_fail(path != NULL, FALSE);

    beatmap = vosu_application_get_beatmap_by_path(app, path);
    if (beatmap != NULL) {
        switch_to_beatmap(app, beatmap);
        return TRUE;
    } else {
        return open_beatmap(app, path);
    }
}

void
vosu_application_set_current_beatmap(VosuApplication *app, VosuBeatmap *bm)
{
    app->current_beatmap = bm;
}

void vosu_application_close_beatmap(VosuApplication *app, VosuBeatmap *beatmap)
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

    VosuApplication *app = VOSU_APPLICATION(app_ptr);
    if (app->current_beatmap != NULL)
        vosu_application_close_beatmap(app, app->current_beatmap);
}

static void
quit_action(GSimpleAction *action, GVariant *parameter, gpointer app)
{
    (void) action;
    (void) parameter;
    g_application_quit(G_APPLICATION(app));
}

static GtkWidget *
create_open_beatmap_dialog(VosuApplication *app, GtkWindow *window)
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
    VosuApplication *app = VOSU_APPLICATION(app_ptr);
    GtkWidget *dialog = create_open_beatmap_dialog(app, GTK_WINDOW(app->window));

    gint res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        gchar *filename = gtk_file_chooser_get_filename(chooser);
        gtk_widget_destroy(dialog);
        vosu_application_open_beatmap(app, filename);
        g_free(filename);
    } else
        gtk_widget_destroy(dialog);
}

static void
new_action(GSimpleAction *action, GVariant *parameter, gpointer app_ptr)
{
    (void) action;
    (void) parameter;
    VosuBeatmap *beatmap = vosu_beatmap_new();
    add_beatmap(VOSU_APPLICATION(app_ptr), beatmap);
}

static void
save_action(GSimpleAction *action, GVariant *parameter, gpointer papp)
{
    (void) action;
    (void) parameter;
    VosuApplication *app = VOSU_APPLICATION(papp);
    VosuBeatmap *beatmap = app->current_beatmap;
    
    if (beatmap == NULL)
        return;

    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new(
        _("Save beatmap"), GTK_WINDOW(app->window), GTK_FILE_CHOOSER_ACTION_SAVE,
        _("_Cancel"), GTK_RESPONSE_CANCEL,
        _("_Save"), GTK_RESPONSE_ACCEPT, NULL);
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
    
    if (beatmap->dirpath != NULL)
        gtk_file_chooser_set_current_folder(chooser, beatmap->dirpath);
    if (beatmap->filename != NULL)
        gtk_file_chooser_set_current_name(chooser, beatmap->filename);
    
    gint res = gtk_dialog_run(GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        filename = gtk_file_chooser_get_filename(chooser);
        vosu_beatmap_save_to_file(app->current_beatmap, filename);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

static GActionEntry app_entries[] = {
    { "quit", &quit_action, NULL, NULL, NULL, {0}},
    { "open", &open_action, NULL, NULL, NULL, {0}},
    { "new", &new_action, NULL, NULL, NULL, {0}},
    { "save", &save_action, NULL, NULL, NULL, {0}},
    { "save_as", &save_action, NULL, NULL, NULL, {0}},
    { "close", &close_action, NULL, NULL, NULL, {0}},
};

/* ---- actions end */

static void
vosu_application_startup(GApplication *gapp)
{
    VosuApplication *app = VOSU_APPLICATION( gapp );
    G_APPLICATION_CLASS(vosu_application_parent_class)->startup(gapp);
    g_action_map_add_action_entries(
        G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);
    app->window = vosu_window_new(app);
}

static void
vosu_application_activate(GApplication *gapp)
{
    VosuApplication *app = VOSU_APPLICATION( gapp );
    gtk_widget_show(GTK_WIDGET(app->window));
    gtk_window_present(GTK_WINDOW(app->window));
}

static void
vosu_application_open(GApplication *gapp, GFile **files,
                       gint n_files, gchar const *hint)
{
    (void) hint;
    VosuApplication *app = VOSU_APPLICATION( gapp );

    for (int i = 0; i < n_files; i++) {
        gchar *path = g_file_get_path(files[i]);
        vosu_application_open_beatmap(app, path);
        g_free(path);
    }
}

static void
vosu_application_class_init(VosuApplicationClass *klass)
{
    G_APPLICATION_CLASS(klass)->startup = vosu_application_startup;
    G_APPLICATION_CLASS(klass)->open = vosu_application_open;
    G_APPLICATION_CLASS(klass)->activate = vosu_application_activate;

    G_OBJECT_CLASS(klass)->dispose = vosu_application_dispose;
    G_OBJECT_CLASS(klass)->finalize = vosu_application_finalize;
}

VosuApplication *
vosu_application_new(void)
{
    return g_object_new(VOSU_TYPE_APPLICATION,
                        "application-id", "org.osux.vosu",
                        "flags", G_APPLICATION_HANDLES_OPEN, NULL);
}

VosuBeatmap *
vosu_application_get_beatmap_by_view(VosuApplication *app, VosuView *view)
{
    GList *l;
    for (l = app->beatmaps; l; l = l->next) {
        if (((VosuBeatmap*) l->data)->view == view)
            return l->data;
    }
    return NULL;
}

VosuBeatmap *
vosu_application_get_beatmap_by_path(VosuApplication *app, gchar const *path)
{
    GList *l;
    for (l = app->beatmaps; l; l = l->next) {
        if (!g_strcmp0(((VosuBeatmap*) l->data)->filepath, path))
            return l->data;
    }
    return NULL;
}
