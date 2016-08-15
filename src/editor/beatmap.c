#include <string.h>
#include "beatmap.h"
#include "osux/beatmap.h"

struct _OsuxEditorBeatmap
{
    GObject parent;
    int error_code;
    char *filename;
    osux_beatmap beatmap;

    GtkAdjustment *audio_leadin;
    GtkAdjustment *beat_divisor;
    GtkAdjustment *preview_time;
    GtkAdjustment *stack_leniency;
    GtkAdjustment *slider_velocity;
    GtkAdjustment *tick_rate;
    GtkAdjustment *timeline_zoom;

    GtkAdjustment *time; // position of the editor in the song;

    GtkAdjustment *approach_rate;
    GtkAdjustment *circle_size;
    GtkAdjustment *overall_difficulty;
    GtkAdjustment *heal_point;

    GtkListStore *objects;

    GtkAdjustment *beatmap_id;
    GtkAdjustment *beatmap_set_id;
};

G_DEFINE_TYPE(OsuxEditorBeatmap, osux_editor_beatmap, G_TYPE_OBJECT);

enum {
    PROP_0 = 0, PROP_FILENAME, N_PROPERTIES
};
static GParamSpec *beatmap_properties[N_PROPERTIES] = {NULL,};

void osux_editor_beatmap_init(OsuxEditorBeatmap *beatmap)
{
    (void) beatmap;
}

static void osux_editor_beatmap_set_property(
    GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    OsuxEditorBeatmap *self = OSUX_EDITOR_BEATMAP( object );
    switch (property_id) {
    case PROP_FILENAME:
        g_free(self->filename);
        self->filename = g_value_dup_string(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
osux_editor_beatmap_constructed(GObject *obj)
{
    int err = 0;
    OsuxEditorBeatmap *beatmap = OSUX_EDITOR_BEATMAP( obj );
    err = osux_beatmap_init(&beatmap->beatmap, beatmap->filename);
    beatmap->error_code = err;
    printf("open_beatmap: %s\n", beatmap->filename);

    if (!err) {
        /* TODO set the various GtkAdjustment* */
    }
    G_OBJECT_CLASS (osux_editor_beatmap_parent_class)->constructed (obj);
}

static void
osux_editor_beatmap_dispose(GObject *obj)
{
    OsuxEditorBeatmap *beatmap = OSUX_EDITOR_BEATMAP( obj );
    g_clear_object(&beatmap->audio_leadin);
    g_clear_object(&beatmap->beat_divisor);
    g_clear_object(&beatmap->preview_time);
    g_clear_object(&beatmap->stack_leniency);
    g_clear_object(&beatmap->slider_velocity);
    g_clear_object(&beatmap->tick_rate);
    g_clear_object(&beatmap->timeline_zoom);
    g_clear_object(&beatmap->time);
    g_clear_object(&beatmap->approach_rate);
    g_clear_object(&beatmap->circle_size);
    g_clear_object(&beatmap->overall_difficulty);
    g_clear_object(&beatmap->heal_point);
    g_clear_object(&beatmap->objects);
    g_clear_object(&beatmap->objects);
    g_clear_object(&beatmap->beatmap_id);
    g_clear_object(&beatmap->beatmap_set_id);
}

static void
osux_editor_beatmap_finalize(GObject *obj)
{
    OsuxEditorBeatmap *beatmap = OSUX_EDITOR_BEATMAP( obj );
    
    if (!beatmap->error_code) {
        osux_beatmap_free(&beatmap->beatmap);
        memset(&beatmap->beatmap, 0, sizeof beatmap->beatmap);
    }
    g_free(beatmap->filename);
    beatmap->filename = NULL;
}

void osux_editor_beatmap_class_init(OsuxEditorBeatmapClass *klass)
{
    GObjectClass *oclass = G_OBJECT_CLASS( klass );
    beatmap_properties[PROP_FILENAME] = g_param_spec_string(
        "filename", "filename", "filename", NULL,
        G_PARAM_WRITABLE|G_PARAM_CONSTRUCT);

    oclass->set_property = osux_editor_beatmap_set_property;
    g_object_class_install_properties(oclass, N_PROPERTIES, beatmap_properties);

    oclass->constructed = osux_editor_beatmap_constructed;
    oclass->dispose = osux_editor_beatmap_dispose;
    oclass->finalize = osux_editor_beatmap_finalize;
}

OsuxEditorBeatmap *osux_editor_beatmap_new(char const *filename)
{
    GObject *object;
    OsuxEditorBeatmap *beatmap;

    object = g_object_new(OSUX_TYPE_EDITOR_BEATMAP, "filename", filename, NULL);
    beatmap = OSUX_EDITOR_BEATMAP( object );
    if (beatmap->error_code < 0) {
        printf("invalid beatmap: %s\n", filename);
        g_object_unref(object);
        return NULL;
    }
    printf("valid beatmap: %s\n", filename);
    return beatmap;
}
