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
vosu_beatmap_dispose(GObject *obj)
{
    VosuBeatmap *beatmap = VOSU_BEATMAP( obj );
    g_clear_object(&beatmap->view);
    G_OBJECT_CLASS(vosu_beatmap_parent_class)->dispose(obj);
    printf("beatmap disposed\n");
}

static void
vosu_beatmap_finalize(GObject *obj)
{
    VosuBeatmap *beatmap = VOSU_BEATMAP( obj );

    g_clear_pointer(&beatmap->filepath, g_free);
    g_clear_pointer(&beatmap->filename, g_free);
    g_clear_pointer(&beatmap->dirpath, g_free);
    g_clear_pointer(&beatmap->errmsg, g_free);
    g_clear_pointer(&beatmap->music_file, g_free);
    osux_beatmap_free(&beatmap->xbeatmap);

    g_clear_pointer(&beatmap->HitObjectsSeq, g_sequence_free);
    G_OBJECT_CLASS(vosu_beatmap_parent_class)->finalize(obj);
    printf("beatmap finalized\n");
}

void
vosu_beatmap_class_init(VosuBeatmapClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS( klass );
    object_class->dispose = vosu_beatmap_dispose;
    object_class->finalize = vosu_beatmap_finalize;
}

VosuBeatmap *vosu_beatmap_new(void)
{
    return VOSU_BEATMAP(g_object_new(VOSU_TYPE_BEATMAP, NULL));
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

static gint
sort_object_end_offset(gconstpointer _a, gconstpointer _b,
                       gpointer UNUSED user_data)
{
    (void) user_data;
    return ((osux_hitobject*)_a)->end_offset - ((osux_hitobject*)_b)->end_offset;
}


static void
load_hit_objects(osux_beatmap *beatmap, GSequence *ho_seq)
{
    for (unsigned i = 0; i < beatmap->hitobject_count; ++i) {
        osux_hitobject *ho = &beatmap->hitobjects[i];
        g_sequence_append(ho_seq, ho);
    }
    g_sequence_sort(ho_seq, &sort_object_end_offset, NULL);
}

gboolean
vosu_beatmap_load_from_file(VosuBeatmap *b, gchar const *filepath)
{
    int err = 0;
    osux_beatmap *pBm = &b->xbeatmap;
    err = osux_beatmap_init(pBm, filepath);

    if (!err) {
        set_path(b, filepath);
        int64_t end_time, beatlength = 1000;
        end_time = pBm->hitobjects[pBm->hitobject_count-1].offset;
        if (pBm->timingpoint_count > 0)
            beatlength = pBm->timingpoints[0].millisecond_per_beat;
        load_hit_objects(pBm, b->HitObjectsSeq);
        b->music_file = g_build_filename(
            b->dirpath, pBm->AudioFilename, NULL);
        vosu_view_set_properties(b->view,
                                 end_time+2000,
                                 beatlength,
                                 b->HitObjectsSeq,
                                 pBm->ApproachRate, 0,
                                 b->music_file);
        return TRUE;
    } else {
        b->errmsg = g_strdup(osux_errmsg(err));
        return FALSE;
    }
}

