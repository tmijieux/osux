#include <gtk/gtk.h>
#include <stdint.h>

#include "app.h"
#include "app_win.h"
#include "beatmap.h"
#include "popsearch.h"
#include "closelabel.h"

G_DEFINE_TYPE(OsuxEditorWindow, osux_editor_window, GTK_TYPE_APPLICATION_WINDOW);

static gboolean osux_editor_window_handle_switch_page(
    GtkNotebook *notebook, GtkWidget *page, guint num_page, gpointer user_data);



#define ADD_FILTER(chooser,builder, filter_name)                        \
    do {                                                                \
        gtk_file_chooser_add_filter(                                    \
            chooser, GTK_FILE_FILTER(                                   \
                gtk_builder_get_object(builder, filter_name)));         \
    } while(0)
        

static void add_music_filters(OsuxEditorWindow *win)
{
    GtkBuilder *builder;
    builder = gtk_builder_new_from_resource(
        "/org/osux/editor/ui/OsuxFileFilterMusic.glade");
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(win->AudioFile);

    ADD_FILTER(chooser, builder, "Audio files");
    ADD_FILTER(chooser, builder, "*.mp3");
    ADD_FILTER(chooser, builder, "*.wma");
    ADD_FILTER(chooser, builder, "*.wav");
    ADD_FILTER(chooser, builder, "*.ogg");
    ADD_FILTER(chooser, builder, "All files");
    g_object_unref(G_OBJECT( builder));
}

static void
osux_editor_window_init(OsuxEditorWindow *win)
{
    gtk_widget_init_template(GTK_WIDGET(win));
    win->popover = GTK_WIDGET(
        osux_editor_popsearch_new(win->new_circle_button) );
    g_signal_connect(win->main_tab, "switch-page",
                     G_CALLBACK(osux_editor_window_handle_switch_page), win);

    add_music_filters(win);
}

static void
osux_editor_window_class_init(OsuxEditorWindowClass *klass)
{
    GtkWidgetClass *wklass = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        wklass, "/org/osux/editor/ui/OsuxEditorWindow.glade");
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, new_circle_button);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, main_tab);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, AudioFile);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, GameMode);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, SampleSet);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, StackLeniency);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, PreviewTime);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, AudioLeadIn);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, CountDown);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, LetterboxInBreaks);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, Widescreen);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, TitleUnicode);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, Title);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, ArtistUnicode);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, Artist);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, Creator);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, Version);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, Source);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, Tags);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, BeatmapID);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, BeatmapSetID);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, DistanceSpacing);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, BeatDivisor);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, GridSize);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, TimelineZoom);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, HPDrainRate);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, CircleSize);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, OverallDifficulty);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, ApproachRate);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, SliderMultiplier);
    gtk_widget_class_bind_template_child(wklass, OsuxEditorWindow, SliderTickRate);
}

OsuxEditorWindow *
osux_editor_window_new(OsuxEditorApp *app)
{
    return g_object_new(OSUX_TYPE_EDITOR_WINDOW, "application", app, NULL);
}

gint osux_editor_window_add_beatmap_tab(
    OsuxEditorWindow *win, OsuxEditorBeatmap *beatmap)
{
    gint c = gtk_notebook_get_current_page(win->main_tab);
    if (c != -1) {
        GtkWidget *label;
        label = gtk_notebook_get_tab_label(win->main_tab, beatmap->main_view);
        if (label != NULL) {
            return;
        }
    }

    gint n_pages = gtk_notebook_get_n_pages(win->main_tab);
    if (n_pages == 1)
        gtk_notebook_set_show_tabs(win->main_tab, TRUE);

    GtkWidget *beatmap_name_label;
    beatmap_name_label = osux_close_label_new(beatmap->beatmap.Title);
    gint pos;
    pos = gtk_notebook_append_page(win->main_tab, beatmap->main_view,
                                   beatmap_name_label);
    gtk_widget_show_all(GTK_WIDGET( win->main_tab ));
    gtk_notebook_set_current_page(win->main_tab, pos);
    return pos;
}

static gboolean osux_editor_window_handle_switch_page(
    GtkNotebook *notebook, GtkWidget *page, guint num_page, gpointer user_data)
{
    (void) notebook;
    (void) num_page;

    OsuxEditorWindow *win = OSUX_EDITOR_WINDOW( user_data );
    OsuxEditorApp *app;
    g_object_get(G_OBJECT( win), "application", &app, NULL);
    OsuxEditorBeatmap *beatmap = osux_editor_app_get_beatmap_by_page(app, page);
    
    if (beatmap == NULL)
        return TRUE;

    // bind current beatmap handles to Widget:
    gtk_window_set_title(GTK_WINDOW(win), beatmap->filename);    
    //general
    gtk_widget_set_sensitive(GTK_WIDGET(win->AudioFile), TRUE);
    gtk_combo_box_set_active(GTK_COMBO_BOX(win->GameMode), beatmap->beatmap.Mode);
    gtk_spin_button_set_adjustment(win->StackLeniency,
                                   GTK_ADJUSTMENT(beatmap->StackLeniency));
    gtk_spin_button_set_adjustment(win->PreviewTime,
                                   GTK_ADJUSTMENT(beatmap->PreviewTime));
    gtk_spin_button_set_adjustment(win->AudioLeadIn,
                                   GTK_ADJUSTMENT(beatmap->AudioLeadIn));
    gtk_spin_button_set_adjustment(win->AudioLeadIn,
                                   GTK_ADJUSTMENT(beatmap->AudioLeadIn));
    gtk_switch_set_active(win->CountDown, beatmap->beatmap.Countdown);
    gtk_switch_set_active(win->LetterboxInBreaks,
                          beatmap->beatmap.LetterboxInBreaks);
    gtk_switch_set_active(win->Widescreen,
                          beatmap->beatmap.WidescreenStoryboard);
    //metadata
    gtk_entry_set_text(win->TitleUnicode, beatmap->beatmap.TitleUnicode);
    gtk_entry_set_text(win->Title, beatmap->beatmap.Title);        
    gtk_entry_set_text(win->ArtistUnicode, beatmap->beatmap.ArtistUnicode);
    gtk_entry_set_text(win->Artist, beatmap->beatmap.Artist);
    gtk_entry_set_text(win->Creator, beatmap->beatmap.Creator);
    gtk_entry_set_text(win->Version, beatmap->beatmap.Version);
    gtk_entry_set_text(win->Source, beatmap->beatmap.Source);
    gtk_spin_button_set_adjustment(win->BeatmapID,
                                   GTK_ADJUSTMENT(beatmap->BeatmapID));
    gtk_spin_button_set_adjustment(win->BeatmapSetID,
                                   GTK_ADJUSTMENT(beatmap->BeatmapSetID));
    //editor
    gtk_spin_button_set_adjustment(win->DistanceSpacing,
                                   GTK_ADJUSTMENT(beatmap->DistanceSpacing));
    gtk_spin_button_set_adjustment(win->BeatDivisor,
                                   GTK_ADJUSTMENT(beatmap->BeatDivisor));
    gtk_spin_button_set_adjustment(win->GridSize,
                                   GTK_ADJUSTMENT(beatmap->GridSize));
    gtk_spin_button_set_adjustment(win->TimelineZoom,
                                   GTK_ADJUSTMENT(beatmap->TimelineZoom));
    // difficulty
    gtk_range_set_adjustment(GTK_RANGE(win->HPDrainRate),
                             GTK_ADJUSTMENT(beatmap->HPDrainRate));
    gtk_range_set_adjustment(GTK_RANGE(win->CircleSize),
                             GTK_ADJUSTMENT(beatmap->CircleSize));
    gtk_range_set_adjustment(GTK_RANGE(win->OverallDifficulty),
                             GTK_ADJUSTMENT(beatmap->OverallDifficulty));
    gtk_range_set_adjustment(GTK_RANGE(win->ApproachRate),
                             GTK_ADJUSTMENT(beatmap->ApproachRate));
    gtk_spin_button_set_adjustment(win->SliderMultiplier,
                                   GTK_ADJUSTMENT(beatmap->SliderMultiplier));
    gtk_spin_button_set_adjustment(win->SliderTickRate,
                                   GTK_ADJUSTMENT(beatmap->SliderTickRate));
        

    gchar *dirpath = g_path_get_dirname(beatmap->filepath);
    gtk_file_chooser_set_current_folder(
        GTK_FILE_CHOOSER(win->AudioFile), dirpath);
    g_free(dirpath);

    gtk_file_chooser_set_filename(
        GTK_FILE_CHOOSER(win->AudioFile), beatmap->audio_filepath);
    return FALSE;
}

gboolean on_main_window_key_press(
    OsuxEditorWindow *win, GdkEventKey *event, gpointer user_data)
{
    (void) user_data;

    GtkWidget *popover = win->popover;
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
