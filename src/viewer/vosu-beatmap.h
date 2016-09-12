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

    VosuView *view;

    GtkTreeStore *Objects;
    GtkTreeIter TimingPoints;
    GtkTreeIter HitObjects;
    GSequence *HitObjectsSeq;

    GtkTreeIter Bookmarks;
    GtkTreeIter Events;
    GtkTreeIter Colors;

    // stolen values when beatmap is loaded:
    osux_hitobject *hitobjects;
    osux_event *events;
    osux_color *colors;
    uint32_t color_count;
    osux_timingpoint *timingpoints;
};

VosuBeatmap *vosu_beatmap_new(gchar const *filepath);
gboolean vosu_beatmap_load_from_file(VosuBeatmap *beatmap, gchar const *filepath);

void vosu_beatmap_load_objects(VosuBeatmap *beatmap, osux_beatmap *osux_bm);

void vosu_beatmap_dispose_objects(VosuBeatmap *beatmap);

G_END_DECLS

#endif //VOSU_BEATMAP_H
