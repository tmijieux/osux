#ifndef EDOSU_BEATMAP_H
#define EDOSU_BEATMAP_H

#include <gtk/gtk.h>
#include "osux/beatmap.h"
#include "edosu-adjust.h"
#include "edosu-view.h"
#include "edosu-palette.h"
#include "edosu-inspector.h"
#include "edosu-properties.h"

G_BEGIN_DECLS

#define EDOSU_TYPE_BEATMAP (edosu_beatmap_get_type())

G_DECLARE_FINAL_TYPE(EdosuBeatmap, edosu_beatmap, EDOSU, BEATMAP, GObject);
struct _EdosuBeatmap
{
    GObject parent;
    char *dirpath; //canonical
    char *filepath; //canonical
    char *filename;
    char *errmsg;
    gint unique_number;

    EdosuView *view;
    EdosuPalette *palette;
    EdosuInspector *inspector;
    EdosuProperties *properties;

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
    osux_timingpoint *timingpoints;
};

EdosuBeatmap *edosu_beatmap_new(void);
gboolean edosu_beatmap_load_from_file(EdosuBeatmap *beatmap, gchar const *filepath);
void edosu_beatmap_save_to_file(EdosuBeatmap *beatmap, gchar const *filepath);

void edosu_beatmap_load_objects(EdosuBeatmap *beatmap, osux_beatmap *osux_bm);
void edosu_beatmap_save_objects(EdosuBeatmap *beatmap, osux_beatmap *osux_bm);
void edosu_beatmap_load_and_bind_adjustments(EdosuBeatmap *beatmap,
                                             osux_beatmap *osux_bm);
void edosu_beatmap_dispose_objects(EdosuBeatmap *beatmap);

G_END_DECLS

#endif //EDOSU_BEATMAP_H
