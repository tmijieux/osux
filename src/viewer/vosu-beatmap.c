#include <glib/gi18n.h>
#include <string.h>

#include "vosu-beatmap.h"
#include "osux/hitobject.h"
#include "osux/error.h"

G_DEFINE_TYPE(VosuBeatmap, vosu_beatmap, G_TYPE_OBJECT);

void
vosu_beatmap_init(VosuBeatmap *beatmap)
{
    beatmap->view = vosu_view_new();
    beatmap->HitObjectsSeq = g_sequence_new(NULL);
}


static void
vosu_beatmap_constructed(GObject *obj)
{
    G_OBJECT_CLASS (vosu_beatmap_parent_class)->constructed (obj);
}

static void
vosu_beatmap_dispose(GObject *obj)
{
    VosuBeatmap *beatmap = VOSU_BEATMAP( obj );

    g_clear_object(&beatmap->view);
    vosu_beatmap_dispose_objects(beatmap);

    G_OBJECT_CLASS(vosu_beatmap_parent_class)->dispose(obj);
}

static void
vosu_beatmap_finalize(GObject *obj)
{
    VosuBeatmap *beatmap = VOSU_BEATMAP( obj );

    g_clear_pointer(&beatmap->filepath, g_free);
    g_clear_pointer(&beatmap->filename, g_free);
    g_clear_pointer(&beatmap->dirpath, g_free);
    g_clear_pointer(&beatmap->errmsg, g_free);


    g_clear_pointer(&beatmap->hitobjects, g_free);
    g_clear_pointer(&beatmap->timingpoints, g_free);
    g_clear_pointer(&beatmap->events, g_free);
    g_clear_pointer(&beatmap->colors, g_free);
    g_clear_pointer(&beatmap->HitObjectsSeq, g_sequence_free);

    G_OBJECT_CLASS(vosu_beatmap_parent_class)->finalize(obj);
}

void
vosu_beatmap_class_init(VosuBeatmapClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS( klass );
    object_class->constructed = vosu_beatmap_constructed;
    object_class->dispose = vosu_beatmap_dispose;
    object_class->finalize = vosu_beatmap_finalize;
}

VosuBeatmap *vosu_beatmap_new(gchar const *filepath)
{
    VosuBeatmap* v;
    v = VOSU_BEATMAP(g_object_new(VOSU_TYPE_BEATMAP, NULL));
    vosu_beatmap_load_from_file(v, filepath);
    return v;
}

static void set_path(VosuBeatmap *beatmap, gchar const *path)
{
    //gchar *canon_path = vosu_util_canonicalize_path(path);
    gchar *canon_path = g_strdup(path);
    g_free(beatmap->filepath);
    g_free(beatmap->filename);
    g_free(beatmap->dirpath);

    beatmap->filepath = canon_path;
    beatmap->dirpath  = g_path_get_dirname(canon_path);
    beatmap->filename = g_path_get_basename(canon_path);
}

gboolean
vosu_beatmap_load_from_file(VosuBeatmap *beatmap, gchar const *filepath)
{
    int err = 0;
    osux_beatmap osux_bm;
    err = osux_beatmap_init(&osux_bm, filepath);

    if (!err) {
        set_path(beatmap, filepath);
        int64_t end_time = osux_bm.hitobjects[osux_bm.hitobject_count-1].offset;
        int64_t beatlength = 1000;
        if (osux_bm.timingpoint_count > 0)
            beatlength = osux_bm.timingpoints[0].millisecond_per_beat;
        vosu_view_set_max_time(beatmap->view, end_time + 5000, beatlength);
        vosu_beatmap_load_objects(beatmap, &osux_bm);
        vosu_view_set_hit_objects(beatmap->view, beatmap->HitObjectsSeq);
        osux_beatmap_free(&osux_bm);
        return TRUE;
    } else {
        beatmap->errmsg = g_strdup(osux_errmsg(err));
        return FALSE;
    }
}
