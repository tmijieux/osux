#include <glib/gi18n.h>
#include <string.h>

#include "edosu-beatmap.h"
#include "edosu-adjust.h"
#include "osux/hitobject.h"

G_DEFINE_TYPE(EdosuBeatmap, edosu_beatmap, G_TYPE_OBJECT);

void
edosu_beatmap_init(EdosuBeatmap *beatmap)
{
    static gint unique_number = 0;
    
    beatmap->view = edosu_view_new();
    beatmap->palette = edosu_palette_new();
    beatmap->inspector = edosu_inspector_new();
    beatmap->properties = edosu_properties_new();
    beatmap->unique_number = unique_number++;
    beatmap->filename = g_strdup_printf(_("unsaved beatmap %d"), unique_number);
}

void
edosu_beatmap_save(EdosuBeatmap *beatmap, gchar const *filepath)
{
    osux_beatmap o_beatmap;
    memset(&o_beatmap, 0, sizeof o_beatmap);

    (void) beatmap;
    // unload_beatmap(beatmap, &o_beatmap);
    osux_beatmap_save(&o_beatmap, filepath);
}

static void
edosu_beatmap_constructed(GObject *obj)
{
    G_OBJECT_CLASS (edosu_beatmap_parent_class)->constructed (obj);
}

static void
edosu_beatmap_dispose(GObject *obj)
{
    EdosuBeatmap *beatmap = EDOSU_BEATMAP( obj );

    g_clear_object(&beatmap->view);
    g_clear_object(&beatmap->palette);
    g_clear_object(&beatmap->inspector);
    g_clear_object(&beatmap->properties);
    G_OBJECT_CLASS (edosu_beatmap_parent_class)->dispose (obj);
}

static void
edosu_beatmap_finalize(GObject *obj)
{
    EdosuBeatmap *beatmap = EDOSU_BEATMAP( obj );

    g_free(beatmap->filepath);
    g_free(beatmap->filename);
    g_free(beatmap->audio_filepath);
    beatmap->filepath = NULL;
    beatmap->filename = NULL;
    beatmap->audio_filepath = NULL;

    G_OBJECT_CLASS (edosu_beatmap_parent_class)->finalize (obj);
}

void
edosu_beatmap_class_init(EdosuBeatmapClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS( klass );
    object_class->constructed = edosu_beatmap_constructed;
    object_class->dispose = edosu_beatmap_dispose;
    object_class->finalize = edosu_beatmap_finalize;
}

EdosuBeatmap *edosu_beatmap_new(void)
{
    return EDOSU_BEATMAP(g_object_new(EDOSU_TYPE_BEATMAP, NULL));
}

static void set_path(EdosuBeatmap *beatmap, gchar const *path)
{
    //gchar *canon_path = edosu_util_canonicalize_path(path);
    gchar *canon_path = g_strdup(path);
    g_free(beatmap->filepath);
    beatmap->filepath = canon_path;
    beatmap->filename = g_path_get_basename(canon_path);
}

gboolean
edosu_beatmap_load_from_file(EdosuBeatmap *beatmap, gchar const *filepath)
{
    int err = 0;
    osux_beatmap osux_bm;
    err = osux_beatmap_init(&osux_bm, filepath);

    if (!err) {
        set_path(beatmap, filepath);
        gchar *dirname;
        dirname = g_path_get_dirname(beatmap->filepath);
        g_free(beatmap->audio_filepath);
        beatmap->audio_filepath = g_build_filename(
            dirname, osux_bm.AudioFilename, NULL);
        g_free(dirname);
        edosu_beatmap_load_objects(beatmap, &osux_bm);
        return TRUE;
    } else {
        return FALSE;
    }
}
