#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdint.h>
#include <glib/gi18n.h>

#include "osux.h"
#include "vosu-app.h"
#include "vosu-win.h"
#include "vosu-beatmap.h"

G_DEFINE_TYPE(VosuApplication, vosu_application, GTK_TYPE_APPLICATION);

static void vosu_application_init(VosuApplication *app)
{
    GtkBuilder *builder;
    GObject *osu_file_filter, *osr_file_filter, *all_file_filter;

    builder = gtk_builder_new_from_resource(
        "/org/osux/vosu/ui/VosuFileFilterOsu.glade");
    all_file_filter = gtk_builder_get_object(builder, "All files");
    osu_file_filter = gtk_builder_get_object(builder, "Beatmaps");
    osr_file_filter = gtk_builder_get_object(builder, "Replays");
    g_object_ref(all_file_filter);
    g_object_ref(osu_file_filter);
    g_object_ref(osr_file_filter);

    app->all_file_filter =  GTK_FILE_FILTER(all_file_filter);
    app->osu_file_filter = GTK_FILE_FILTER(osu_file_filter);
    app->osr_file_filter = GTK_FILE_FILTER(osr_file_filter);

    g_object_unref(G_OBJECT(builder));
    app->beatmap = NULL;
}

static void vosu_application_dispose(GObject *obj)
{
    VosuApplication *app = VOSU_APPLICATION( obj );
    g_clear_object(&app->osu_file_filter);
    g_clear_object(&app->osr_file_filter);
    g_clear_object(&app->all_file_filter);
    g_clear_object(&app->beatmap);

    G_OBJECT_CLASS(vosu_application_parent_class)->dispose(obj);
}

void
vosu_application_set_replay_file(VosuApplication *app,
                                 gchar const *filename)
{
    printf("replay file set to '%s'\n", filename);

    (void) app;
    osux_debug("NOT IMPLEMENTED\n");
}

static void vosu_application_finalize(GObject *obj)
{
    G_OBJECT_CLASS(vosu_application_parent_class)->finalize(obj);
}

static void
vosu_application_error(VosuApplication *app,
                        gchar const *title, gchar const *error)
{
    GtkDialog *dialog;
    dialog = GTK_DIALOG(gtk_message_dialog_new(
        GTK_WINDOW(app->window), GTK_DIALOG_MODAL,
        GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", error));
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(dialog);
    gtk_widget_destroy(GTK_WIDGET(dialog));
    osux_error("%s\n", error);
}

static void
set_beatmap(VosuApplication *app, VosuBeatmap *beatmap)
{
    if (app->beatmap != NULL)
        vosu_application_close_beatmap(app);
    app->beatmap = beatmap;
    osux_debug("prepare to call window set view\n");
    vosu_window_set_view(app->window, beatmap->view);
}

static gboolean
open_beatmap(VosuApplication *app, gchar *path)
{
    VosuBeatmap *beatmap;
    beatmap = vosu_beatmap_new();
    if (beatmap == NULL) {
        return FALSE;
    }
    if (!vosu_beatmap_load_from_file(beatmap, path)) {
        vosu_application_error(app, _("Error loading beatmap"),
                               beatmap->errmsg);
        g_object_unref(beatmap);
        return FALSE;
    }

    set_beatmap(app, beatmap);
    return TRUE;
}

gboolean
vosu_application_open_beatmap(VosuApplication *app, gchar *path)
{
    g_return_val_if_fail(path != NULL, FALSE);
    if (app->beatmap != NULL && !g_strcmp0(path, app->beatmap->filepath))
    {
        gtk_window_present(GTK_WINDOW(app->window));
        return TRUE;
    } else
        return open_beatmap(app, path);
}

void vosu_application_close_beatmap(VosuApplication *app)
{
    vosu_window_close_view(app->window);
    g_object_unref(app->beatmap);
    app->beatmap = NULL;
}

/* ---- application actions */

static void
close_action(GSimpleAction *action, GVariant *parameter, gpointer app_ptr)
{
    (void) action;
    (void) parameter;

    VosuApplication *app = VOSU_APPLICATION(app_ptr);
    if (app->beatmap != NULL)
        vosu_application_close_beatmap(app);
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

static GActionEntry app_entries[] = {
    { "quit", &quit_action, NULL, NULL, NULL, {0}},
    { "open", &open_action, NULL, NULL, NULL, {0}},
    { "close", &close_action, NULL, NULL, NULL, {0}},
};

/*  ---- app actions end  */
/*  ---- window actions  */

static void
fullscreen_action(GSimpleAction *action,
                  GVariant *parameter, gpointer win_ptr)
{
    (void) action;
    (void) parameter;
    VosuWindow *win = VOSU_WINDOW(win_ptr);
    if (!win->fullscreen)
        gtk_window_fullscreen(GTK_WINDOW(win));
    else
        gtk_window_unfullscreen(GTK_WINDOW(win));
}

static GActionEntry win_entries[] = {
    { "fullscreen", &fullscreen_action, NULL, NULL, NULL, {0}},
};

/* ---- window actions end */


/* signal tracking window fullscreen state: */
gboolean
on_window_state_event (GtkWidget *window_widget,
                       GdkEvent  *event)
{
    VosuWindow *win = VOSU_WINDOW(window_widget);
    switch (event->type)  {
    case GDK_WINDOW_STATE: {
        GdkEventWindowState *winev;
        winev = (GdkEventWindowState*) event;
        if (winev->new_window_state & GDK_WINDOW_STATE_FULLSCREEN)
            win->fullscreen = TRUE;
        else
            win->fullscreen = FALSE;
        break;
    }
    default:
        break;
    }
    return FALSE;
}

static void
vosu_application_startup(GApplication *gapp)
{
    VosuApplication *app = VOSU_APPLICATION( gapp );
    G_APPLICATION_CLASS(vosu_application_parent_class)->startup(gapp);
    g_action_map_add_action_entries(
        G_ACTION_MAP(app), app_entries, G_N_ELEMENTS(app_entries), app);
    app->window = vosu_window_new(app);
    g_action_map_add_action_entries(G_ACTION_MAP(app->window), win_entries,
                                    G_N_ELEMENTS(win_entries), app->window);

    g_signal_connect(G_OBJECT(app->window), "window-state-event",
                     G_CALLBACK(on_window_state_event), NULL);
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
