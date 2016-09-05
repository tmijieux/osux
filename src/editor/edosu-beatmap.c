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
    beatmap->HitObjectsSeq = g_sequence_new(NULL);
    beatmap->unique_number = unique_number++;
    beatmap->filename = g_strdup_printf(_("unsaved beatmap %d"), unique_number);
}

void
edosu_beatmap_save_to_file(EdosuBeatmap *beatmap, gchar const *filepath)
{
    osux_beatmap o_beatmap;
    memset(&o_beatmap, 0, sizeof o_beatmap);
    o_beatmap.osu_version = 15;

    edosu_properties_save_to_beatmap(beatmap->properties, &o_beatmap);
    edosu_beatmap_save_objects(beatmap, &o_beatmap);

    osux_beatmap_save(&o_beatmap, filepath);
    osux_beatmap_free(&o_beatmap);
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
    edosu_beatmap_dispose_objects(beatmap);

    G_OBJECT_CLASS(edosu_beatmap_parent_class)->dispose(obj);
}

static void
edosu_beatmap_finalize(GObject *obj)
{
    EdosuBeatmap *beatmap = EDOSU_BEATMAP( obj );

    g_clear_pointer(&beatmap->filepath, g_free);
    g_clear_pointer(&beatmap->filename, g_free);
    g_clear_pointer(&beatmap->dirpath, g_free);
    g_clear_pointer(&beatmap->errmsg, g_free);


    g_clear_pointer(&beatmap->hitobjects, g_free);
    g_clear_pointer(&beatmap->timingpoints, g_free);
    g_clear_pointer(&beatmap->events, g_free);
    g_clear_pointer(&beatmap->colors, g_free);
    g_clear_pointer(&beatmap->HitObjectsSeq, g_sequence_free);

    G_OBJECT_CLASS(edosu_beatmap_parent_class)->finalize(obj);
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
    return EDOSU_BEATMAP(g_object_ref(g_object_new(EDOSU_TYPE_BEATMAP, NULL)));
}

static void set_path(EdosuBeatmap *beatmap, gchar const *path)
{
    //gchar *canon_path = edosu_util_canonicalize_path(path);
    gchar *canon_path = g_strdup(path);
    g_free(beatmap->filepath);
    g_free(beatmap->filename);
    g_free(beatmap->dirpath);

    beatmap->filepath = canon_path;
    beatmap->dirpath  = g_path_get_dirname(canon_path);
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
        int64_t end_time = osux_bm.hitobjects[osux_bm.hitobject_count-1].offset;
        int64_t beatlength = 1000;
        if(osux_bm.timingpoint_count > 0)
            beatlength = osux_bm.timingpoints[0].millisecond_per_beat;
        edosu_view_set_max_time(beatmap->view, end_time + 5000, beatlength);
        edosu_beatmap_load_objects(beatmap, &osux_bm);
        edosu_view_set_hit_objects(beatmap->view, beatmap->HitObjectsSeq);
        edosu_inspector_set_model(beatmap->inspector,
                                  GTK_TREE_MODEL(beatmap->Objects));
        edosu_properties_load_from_beatmap(beatmap->properties, &osux_bm);
        osux_beatmap_free(&osux_bm);
        return TRUE;
    } else {
        beatmap->errmsg = g_strdup(osux_errmsg(err));
        return FALSE;
    }
}
