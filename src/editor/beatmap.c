#include <string.h>
#include "beatmap.h"

G_DEFINE_TYPE(OsuxEditorBeatmap, osux_editor_beatmap, G_TYPE_OBJECT);

enum {
    PROP_0 = 0, PROP_FILEPATH, N_PROPERTIES
};

static GParamSpec *beatmap_properties[N_PROPERTIES] = {NULL,};


#define IMPORT_ADJUSTMENT(builder, beatmap__, field)                    \
    do {                                                                \
        (beatmap)->field = OSUX_ADJUSTMENT(                             \
            gtk_builder_get_object(builder, #field));                   \
        g_object_ref(G_OBJECT( (beatmap)->field ));                     \
        osux_adjustment_set_target((beatmap__)->field,                  \
                                   &((beatmap__)->beatmap.field));      \
    } while(0)

#define IMPORT_ADJUSTMENT_I(builder, beatmap__, field)                  \
    do {                                                                \
        (beatmap)->field = OSUX_ADJUSTMENT(                             \
            gtk_builder_get_object(builder, #field));                   \
        g_object_ref(G_OBJECT( (beatmap)->field ));                     \
        osux_adjustment_set_target_i((beatmap__)->field,                \
                                     &((beatmap__)->beatmap.field));    \
    } while(0)


void
osux_editor_beatmap_init(OsuxEditorBeatmap *beatmap)
{
    beatmap->main_view = gtk_image_new_from_icon_name(
        "gtk-missing-image", GTK_ICON_SIZE_MENU);
}

static void load_and_bind_adjustments(OsuxEditorBeatmap *beatmap)
{
    GtkBuilder *builder;
    builder = gtk_builder_new_from_resource(
        "/org/osux/editor/ui/OsuxBeatmapAdjustments.glade");
    
    //general
    IMPORT_ADJUSTMENT_I(builder, beatmap, BeatmapID);
    IMPORT_ADJUSTMENT_I(builder, beatmap, BeatmapSetID);
    IMPORT_ADJUSTMENT_I(builder, beatmap, AudioLeadIn);
    IMPORT_ADJUSTMENT_I(builder, beatmap, PreviewTime);
    IMPORT_ADJUSTMENT(builder, beatmap, StackLeniency);

    //difficulty
    IMPORT_ADJUSTMENT(builder, beatmap, ApproachRate);
    IMPORT_ADJUSTMENT(builder, beatmap, CircleSize);
    IMPORT_ADJUSTMENT(builder, beatmap, OverallDifficulty);
    IMPORT_ADJUSTMENT(builder, beatmap, HPDrainRate);
    IMPORT_ADJUSTMENT(builder, beatmap, SliderMultiplier);
    IMPORT_ADJUSTMENT(builder, beatmap, SliderTickRate);
    
    //editor:
    IMPORT_ADJUSTMENT_I(builder, beatmap, BeatDivisor);    
    IMPORT_ADJUSTMENT_I(builder, beatmap, GridSize);
    IMPORT_ADJUSTMENT(builder, beatmap, TimelineZoom);
    IMPORT_ADJUSTMENT(builder, beatmap, DistanceSpacing);

    g_object_unref(G_OBJECT( builder ));
}

static void
osux_editor_beatmap_set_property(GObject *object, guint property_id,
                                 const GValue *value, GParamSpec *pspec)
{
    OsuxEditorBeatmap *self = OSUX_EDITOR_BEATMAP( object );
    switch (property_id) {
    case PROP_FILEPATH:
        g_free(self->filepath);
        self->filepath = g_value_dup_string(value);
        self->filename = g_path_get_basename(self->filepath);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

void
osux_editor_beatmap_save(OsuxEditorBeatmap *beatmap)
{
    osux_beatmap_save(&beatmap->beatmap, NULL, true);
}

static void
osux_editor_beatmap_constructed(GObject *obj)
{
    int err = 0;
    OsuxEditorBeatmap *beatmap = OSUX_EDITOR_BEATMAP( obj );
    err = osux_beatmap_init(&beatmap->beatmap, beatmap->filepath);
    beatmap->error_code = err;
    gchar *dirname = g_path_get_dirname(beatmap->filepath);
    beatmap->audio_filepath = g_build_filename(
        dirname, beatmap->beatmap.AudioFilename, NULL);
    g_free(dirname);

    if (!err)
        load_and_bind_adjustments(beatmap);

    G_OBJECT_CLASS (osux_editor_beatmap_parent_class)->constructed (obj);
}

static void
osux_editor_beatmap_dispose(GObject *obj)
{
    OsuxEditorBeatmap *beatmap = OSUX_EDITOR_BEATMAP( obj );
    g_clear_object(&beatmap->Time);
    g_clear_object(&beatmap->Objects);

    g_clear_object(&beatmap->AudioLeadIn); // general
    g_clear_object(&beatmap->PreviewTime);
    g_clear_object(&beatmap->StackLeniency);
    g_clear_object(&beatmap->BeatmapID);
    g_clear_object(&beatmap->BeatmapSetID);
    
    g_clear_object(&beatmap->ApproachRate); //difficulty
    g_clear_object(&beatmap->CircleSize);
    g_clear_object(&beatmap->OverallDifficulty);
    g_clear_object(&beatmap->HPDrainRate);
    g_clear_object(&beatmap->SliderMultiplier);
    g_clear_object(&beatmap->SliderTickRate);
    
    g_clear_object(&beatmap->BeatDivisor); // editor
    g_clear_object(&beatmap->GridSize);
    g_clear_object(&beatmap->TimelineZoom);
    g_clear_object(&beatmap->DistanceSpacing);

    G_OBJECT_CLASS (osux_editor_beatmap_parent_class)->dispose (obj);
}

static void
osux_editor_beatmap_finalize(GObject *obj)
{
    OsuxEditorBeatmap *beatmap = OSUX_EDITOR_BEATMAP( obj );

    if (!beatmap->error_code) {
        osux_beatmap_free(&beatmap->beatmap);
        memset(&beatmap->beatmap, 0, sizeof beatmap->beatmap);
    }
    g_free(beatmap->filepath);
    g_free(beatmap->filename);
    g_free(beatmap->audio_filepath);
    beatmap->filepath = NULL;
    beatmap->filename = NULL;
    beatmap->audio_filepath = NULL;

    G_OBJECT_CLASS (osux_editor_beatmap_parent_class)->finalize (obj);
}

void
osux_editor_beatmap_class_init(OsuxEditorBeatmapClass *klass)
{
    GObjectClass *oclass = G_OBJECT_CLASS( klass );
    beatmap_properties[PROP_FILEPATH] = g_param_spec_string(
        "filepath", "filepath", "filepath", NULL,
        G_PARAM_WRITABLE|G_PARAM_CONSTRUCT);

    oclass->set_property = osux_editor_beatmap_set_property;
    g_object_class_install_properties(oclass, N_PROPERTIES, beatmap_properties);

    oclass->constructed = osux_editor_beatmap_constructed;
    oclass->dispose = osux_editor_beatmap_dispose;
    oclass->finalize = osux_editor_beatmap_finalize;
}

OsuxEditorBeatmap *
osux_editor_beatmap_new(char const *filepath)
{
    GObject *object;
    OsuxEditorBeatmap *beatmap;

    object = g_object_new(OSUX_TYPE_EDITOR_BEATMAP, "filepath", filepath, NULL);
    beatmap = OSUX_EDITOR_BEATMAP( object );
    if (beatmap->error_code < 0) {
        printf("invalid beatmap: %s\n", filepath);
        g_object_unref(object);
        return NULL;
    }
    printf("valid beatmap: %s\n", filepath);
    return beatmap;
}
