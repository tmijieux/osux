#ifndef VOSU_BEATMAP_H
#define VOSU_BEATMAP_H

#include <gtk/gtk.h>
#include "osux/beatmap.h"
#include "vosu-view.h"

G_BEGIN_DECLS

#define VOSU_TYPE_BEATMAP (vosu_beatmap_get_type())

G_DECLARE_FINAL_TYPE(VosuBeatmap, vosu_beatmap, VOSU, BEATMAP, GObject);
struct _VosuBeatmap
{
    GObject parent;
    char *dirpath; //canonical
    char *filepath; //canonical
    char *filename;
    char *errmsg;
    char *music_file;

    VosuView *view;
    GSequence *HitObjectsSeq;
    osux_beatmap xbeatmap;
};

VosuBeatmap *vosu_beatmap_new(void);
gboolean vosu_beatmap_load_from_file(VosuBeatmap *beatmap, gchar const *filepath);

G_END_DECLS

#endif //VOSU_BEATMAP_H
