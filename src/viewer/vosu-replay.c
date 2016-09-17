#include <glib/gi18n.h>
#include <string.h>

#include "vosu-replay.h"
#include "osux/hitobject.h"
#include "osux/error.h"

G_DEFINE_TYPE(VosuReplay, vosu_replay, G_TYPE_OBJECT);

void
vosu_replay_init(VosuReplay *replay)
{
    replay->CursorDataSeq = vosu_sequence_new(NULL);
}

static void
vosu_replay_dispose(GObject *obj)
{
    VosuReplay *replay = VOSU_REPLAY(obj);
    g_clear_object(&replay->CursorDataSeq);
    G_OBJECT_CLASS(vosu_replay_parent_class)->dispose(obj);
}

static void
vosu_replay_finalize(GObject *obj)
{
    VosuReplay *replay = VOSU_REPLAY(obj);
    osux_replay_free(&replay->xreplay);
    G_OBJECT_CLASS(vosu_replay_parent_class)->finalize(obj);
}

void
vosu_replay_class_init(VosuReplayClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = &vosu_replay_finalize;
    G_OBJECT_CLASS(klass)->dispose = &vosu_replay_dispose;
}

VosuReplay *vosu_replay_new(void)
{
    return VOSU_REPLAY(g_object_new(VOSU_TYPE_REPLAY, NULL));
}

static gint
sort_data_end_offset(gconstpointer a, gconstpointer b, gpointer _)
{
    (void) _;
    return ((osux_replay_data*)a)->time_offset
        - ((osux_replay_data*)b)->time_offset;
}

static void
load_cursor_data(osux_replay *replay, VosuSequence *CursorDataSeq)
{
    for (unsigned i = 0; i < replay->data_count; ++i) {
        osux_replay_data *d = &replay->data[i];
        vosu_sequence_append(CursorDataSeq, d);
    }
    vosu_sequence_sort(CursorDataSeq, &sort_data_end_offset, NULL);
}

gboolean
vosu_replay_load_from_file(VosuReplay *replay, gchar const *filepath)
{
    int err = 0;
    osux_replay *pR = &replay->xreplay;
    err = osux_replay_init(pR, filepath);

    if (!err) {
        load_cursor_data(pR, replay->CursorDataSeq);
        return TRUE;
    } else {
        replay->errmsg = g_strdup(osux_errmsg(err));
        return FALSE;
    }
}

void vosu_replay_configure_view(VosuReplay *replay, VosuView *view)
{
    vosu_view_set_replay_properties(
        view, replay->CursorDataSeq, replay->xreplay.mods);
}

gchar const *
vosu_replay_get_errmsg(VosuReplay *replay)
{
    return replay->errmsg;
}

gchar const *
vosu_replay_get_beatmap_hash(VosuReplay *replay)
{
    return replay->xreplay.beatmap_hash;
}
