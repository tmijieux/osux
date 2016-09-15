#ifndef VOSU_REPLAY_H
#define VOSU_REPLAY_H

#include <gtk/gtk.h>
#include "osux/replay.h"

G_BEGIN_DECLS

#define VOSU_TYPE_REPLAY (vosu_replay_get_type())
G_DECLARE_FINAL_TYPE(VosuReplay, vosu_replay, VOSU, REPLAY, GObject);

#include "vosu-view.h"
#include "vosu-sequence.h"

struct _VosuReplay
{
    GObject parent;
    VosuSequence *CursorDataSeq;
    osux_replay xreplay;
    char *errmsg;
};

VosuReplay *vosu_replay_new(void);
gboolean vosu_replay_load_from_file(VosuReplay *replay,
                                    gchar const *filepath);
void vosu_replay_configure_view(VosuReplay *b, VosuView *view);
gchar const *vosu_replay_get_errmsg(VosuReplay *replay);
gchar const *vosu_replay_get_beatmap_hash(VosuReplay *replay);

G_END_DECLS

#endif //VOSU_REPLAY_H
