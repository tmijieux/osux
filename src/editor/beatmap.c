#include <glib/gi18n.h>
#include <string.h>
#include "beatmap.h"
#include "osux/hitobject.h"

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
        "/org/osux/editor/ui/OsuxBeatmapAdjustments.ui");

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

enum { COL_OFFSET = 0, COL_TYPE, COL_HITSOUND, COL_NUM, };

static void
load_hit_objects(osux_beatmap *beatmap, GtkTreeStore *tree_store,
                 GtkTreeIter *hitobjects)
{
    for (unsigned i = 0; i < beatmap->hitobject_count; ++i)
    {
        osux_hitobject *ho = &beatmap->hitobjects[i];
        char *type;
        char hitsound[20];

        switch (HIT_OBJECT_TYPE(ho)) {
        case HITOBJECT_CIRCLE: type = _("Circle");  break;
        case HITOBJECT_SLIDER: type = _("Slider");  break;
        case HITOBJECT_SPINNER:type = _("Spinner");  break;
        case HITOBJECT_HOLD:   type = _("Hold");  break;
        default: type = _("Invalid type"); break;
        }
        sprintf(hitsound, "%d", ho->hitsound.sample);

        GtkTreeIter iter;
        gtk_tree_store_append(tree_store, &iter, hitobjects);
        gtk_tree_store_set(tree_store, &iter,
                           COL_OFFSET, ho->offset,
                           COL_TYPE, type,
                           COL_HITSOUND, hitsound, -1);
    }
}

static void
load_timing_points(osux_beatmap *beatmap, GtkTreeStore *tree_store,
                   GtkTreeIter *timingpoints)
{
    for (unsigned i = 0; i < beatmap->timingpoint_count; ++i)
    {
        osux_timingpoint *tp = &beatmap->timingpoints[i];
        char *type;
        if (tp->inherited)
            type = _("Inherited");
        else
            type = _("Absolute");

        GtkTreeIter iter;
        gtk_tree_store_append(tree_store, &iter, timingpoints);
        gtk_tree_store_set(tree_store, &iter,
                           COL_OFFSET, (int) tp->offset,
                           COL_TYPE, type,
                           COL_HITSOUND, "", -1);
    }
}

static void
load_colors(osux_beatmap *beatmap, GtkTreeStore *tree_store, GtkTreeIter *colors)
{
    for (unsigned i = 0; i < beatmap->color_count; ++i)
    {
        osux_color *c = &beatmap->colors[i];
        GtkTreeIter iter;
        char color[50];
        sprintf(color, "%d,%d,%d", c->r, c->g, c->b);
        gtk_tree_store_append(tree_store, &iter, colors);
        gtk_tree_store_set(tree_store, &iter,
                           COL_OFFSET, c->id,
                           COL_TYPE, osux_color_type_get_name(c->type),
                           COL_HITSOUND, color, -1);
    }
}

static void load_event(GtkTreeStore *tree_store, osux_event *event,
                       GtkTreeIter *parent_object)
{
    GtkTreeIter iter;
    gtk_tree_store_append(tree_store, &iter, parent_object);
    gtk_tree_store_set(tree_store, &iter,
                       COL_OFFSET, event->offset,
                       COL_TYPE, osux_event_type_get_name(event->type),
                       COL_HITSOUND, "", -1);
    for (unsigned i = 0; i < event->child_count; ++i)
        load_event(tree_store, event->childs[i], &iter);
}

static void
load_events(osux_beatmap *beatmap, GtkTreeStore *tree_store, GtkTreeIter *events)
{
    for (unsigned i = 0; i < beatmap->event_count; ++i) {
        if (!osux_event_is_object(&beatmap->events[i]))
            continue;
        load_event(tree_store, &beatmap->events[i], events);
    }
}

static void
load_bookmarks(osux_beatmap *beatmap, GtkTreeStore *tree_store,
               GtkTreeIter *bookmarks)
{
    for (unsigned i = 0; i < beatmap->bookmark_count; ++i)
    {
        int64_t b = beatmap->bookmarks[i];
        GtkTreeIter iter;
        gtk_tree_store_append(tree_store, &iter, bookmarks);
        gtk_tree_store_set(tree_store, &iter,
                           COL_OFFSET, b,
                           COL_TYPE, _("Bookmark"),
                           COL_HITSOUND, "", -1);
    }
}

static void load_objects(OsuxEditorBeatmap *beatmap)
{
    GtkTreeStore *ts;
    ts = gtk_tree_store_new(COL_NUM, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING);

    beatmap->Objects = ts;
    gtk_tree_store_append(ts, &beatmap->TimingPoints, NULL);
    gtk_tree_store_set(ts, &beatmap->TimingPoints, COL_TYPE, _("TimingPoints"),
                       COL_OFFSET, beatmap->beatmap.timingpoint_count, -1);
    load_timing_points(&beatmap->beatmap, ts, &beatmap->TimingPoints);

    gtk_tree_store_append(ts, &beatmap->HitObjects, NULL);
    gtk_tree_store_set(ts, &beatmap->HitObjects, COL_TYPE, _("HitObjects"),
                       COL_OFFSET, beatmap->beatmap.hitobject_count, -1);
    load_hit_objects(&beatmap->beatmap, ts, &beatmap->HitObjects);

    gtk_tree_store_append(ts, &beatmap->Bookmarks, NULL);
    gtk_tree_store_set(ts, &beatmap->Bookmarks, COL_TYPE, _("Bookmarks"),
                       COL_OFFSET, beatmap->beatmap.bookmark_count, -1);
    load_bookmarks(&beatmap->beatmap, ts, &beatmap->Bookmarks);

    gtk_tree_store_append(ts, &beatmap->Events, NULL);
    gtk_tree_store_set(ts, &beatmap->Events, COL_TYPE, _("Events"),
                       COL_OFFSET, beatmap->beatmap.event_count, -1);
    load_events(&beatmap->beatmap, ts, &beatmap->Events);

    gtk_tree_store_append(ts, &beatmap->Colors, NULL);
    gtk_tree_store_set(ts, &beatmap->Colors, COL_TYPE, _("Colors"),
                       COL_OFFSET, beatmap->beatmap.color_count, -1);
    load_colors(&beatmap->beatmap, ts, &beatmap->Colors);
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
    load_objects(beatmap);

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
        printf(_("invalid beatmap: %s\n"), filepath);
        g_object_unref(object);
        return NULL;
    }
    printf(_("valid beatmap: %s\n"), filepath);
    return beatmap;
}
