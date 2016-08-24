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
    char *filepath; //canonical
    char *filename;
    char *audio_filepath;
    gint unique_number;

    EdosuView *view;
    EdosuPalette *palette;
    EdosuInspector *inspector;
    EdosuProperties *properties;
    
    GtkTreeStore *Objects;
    GtkTreeIter TimingPoints;
    GtkTreeIter HitObjects;
    GtkTreeIter Bookmarks;
    GtkTreeIter Events;
    GtkTreeIter Colors;
};

EdosuBeatmap *edosu_beatmap_new(void);
gboolean edosu_beatmap_load_from_file(EdosuBeatmap *beatmap, gchar const *filepath);
void edosu_beatmap_save(EdosuBeatmap *beatmap, gchar const *filepath);

void edosu_beatmap_load_objects(EdosuBeatmap *beatmap, osux_beatmap *osux_bm);
void edosu_beatmap_load_and_bind_adjustments(EdosuBeatmap *beatmap,
                                             osux_beatmap *osux_bm);

G_END_DECLS

#endif //EDOSU_BEATMAP_H
