#include <gtk/gtk.h>
#include <stdint.h>

#include "edosu-properties.h"

G_DEFINE_TYPE(EdosuProperties, edosu_properties, GTK_TYPE_NOTEBOOK);


#define ADD_FILTER(chooser,builder, filter_name)                \
    do {                                                        \
        gtk_file_chooser_add_filter(                            \
            chooser, GTK_FILE_FILTER(                           \
                gtk_builder_get_object(builder, filter_name))); \
    } while(0)


static void
add_music_filters(GtkFileChooser *chooser)
{
    GtkBuilder *builder;
    builder = gtk_builder_new_from_resource(
        "/org/osux/edosu/ui/EdosuFileFilterMusic.glade");

    ADD_FILTER(chooser, builder, "Audio files");
    ADD_FILTER(chooser, builder, "*.mp3");
    ADD_FILTER(chooser, builder, "*.wma");
    ADD_FILTER(chooser, builder, "*.wav");
    ADD_FILTER(chooser, builder, "*.ogg");
    ADD_FILTER(chooser, builder, "All files");
    g_object_unref(G_OBJECT( builder));
}

static void
edosu_properties_init(EdosuProperties *props)
{
    gtk_widget_init_template(GTK_WIDGET(props));
    add_music_filters(props->audio_file_chooser);
}

static void
edosu_properties_class_init(EdosuPropertiesClass *klass)
{
    GtkWidgetClass *k = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        k, "/org/osux/edosu/ui/EdosuProperties.glade");

    gtk_widget_class_bind_template_child(k, EdosuProperties, audio_file_chooser);
    gtk_widget_class_bind_template_child(k, EdosuProperties, Mode);
    gtk_widget_class_bind_template_child(k, EdosuProperties, SampleSet);
    gtk_widget_class_bind_template_child(k, EdosuProperties, StackLeniency);
    gtk_widget_class_bind_template_child(k, EdosuProperties, PreviewTime);
    gtk_widget_class_bind_template_child(k, EdosuProperties, AudioLeadIn);
    gtk_widget_class_bind_template_child(k, EdosuProperties, Countdown);
    gtk_widget_class_bind_template_child(k, EdosuProperties, LetterboxInBreaks);
    gtk_widget_class_bind_template_child(k, EdosuProperties, WidescreenStoryboard);

    gtk_widget_class_bind_template_child(k, EdosuProperties, TitleUnicode);
    gtk_widget_class_bind_template_child(k, EdosuProperties, Title);
    gtk_widget_class_bind_template_child(k, EdosuProperties, ArtistUnicode);
    gtk_widget_class_bind_template_child(k, EdosuProperties, Artist);
    gtk_widget_class_bind_template_child(k, EdosuProperties, Creator);
    gtk_widget_class_bind_template_child(k, EdosuProperties, Version);
    gtk_widget_class_bind_template_child(k, EdosuProperties, Source);
    gtk_widget_class_bind_template_child(k, EdosuProperties, Tags);
    gtk_widget_class_bind_template_child(k, EdosuProperties, BeatmapID);
    gtk_widget_class_bind_template_child(k, EdosuProperties, BeatmapSetID);

    gtk_widget_class_bind_template_child(k, EdosuProperties, DistanceSpacing);
    gtk_widget_class_bind_template_child(k, EdosuProperties, BeatDivisor);
    gtk_widget_class_bind_template_child(k, EdosuProperties, GridSize);
    gtk_widget_class_bind_template_child(k, EdosuProperties, TimelineZoom);

    gtk_widget_class_bind_template_child(k, EdosuProperties, HPDrainRate);
    gtk_widget_class_bind_template_child(k, EdosuProperties, CircleSize);
    gtk_widget_class_bind_template_child(k, EdosuProperties, OverallDifficulty);
    gtk_widget_class_bind_template_child(k, EdosuProperties, ApproachRate);
    gtk_widget_class_bind_template_child(k, EdosuProperties, SliderMultiplier);
    gtk_widget_class_bind_template_child(k, EdosuProperties, SliderTickRate);
}

EdosuProperties *
edosu_properties_new(void)
{
    return EDOSU_PROPERTIES(g_object_new(EDOSU_TYPE_PROPERTIES, NULL));
}

#define LOAD_ADJUST(field)                              \
    gtk_adjustment_set_value(p->field, beatmap->field);

#define LOAD_BOOL(field)                                \
    gtk_switch_set_active(p->field, beatmap->field);

#define LOAD_TEXT(field)                                \
    gtk_entry_set_text(p->field, beatmap->field);

#define LOAD_ENUM(field)                                                \
    gtk_combo_box_set_active(GTK_COMBO_BOX(p->field), beatmap->field);

void
edosu_properties_load_from_beatmap(EdosuProperties *p, osux_beatmap *beatmap)
{
    LOAD_ADJUST(BeatmapID);
    LOAD_ADJUST(BeatmapSetID);
    LOAD_ADJUST(DistanceSpacing);
    LOAD_ADJUST(BeatDivisor);
    LOAD_ADJUST(GridSize);
    LOAD_ADJUST(TimelineZoom);
    LOAD_ADJUST(HPDrainRate);
    LOAD_ADJUST(CircleSize);
    LOAD_ADJUST(OverallDifficulty);
    LOAD_ADJUST(ApproachRate);
    LOAD_ADJUST(SliderMultiplier);
    LOAD_ADJUST(SliderTickRate);

    LOAD_BOOL(Countdown);
    LOAD_BOOL(LetterboxInBreaks);
    LOAD_BOOL(WidescreenStoryboard);

    LOAD_TEXT(TitleUnicode);
    LOAD_TEXT(Title);
    LOAD_TEXT(ArtistUnicode);
    LOAD_TEXT(Artist);
    LOAD_TEXT(Creator);
    LOAD_TEXT(Version);
    LOAD_TEXT(Source);
    LOAD_TEXT(Tags);

    LOAD_ENUM(Mode);
    //LOAD_ENUM(SampleSet);

    gchar *dirname, *audio_path;
    dirname = g_path_get_dirname(beatmap->file_path);
    audio_path = g_build_filename(dirname, beatmap->AudioFilename, NULL);
    gtk_file_chooser_set_filename(p->audio_file_chooser, audio_path);
    g_free(dirname);
    g_free(audio_path);
}
