#ifndef OSUX_EDITOR_BEATMAP_H
#define OSUX_EDITOR_BEATMAP_H

#include <gtk/gtk.h>
#include "osux/beatmap.h"
#include "adjustment.h"

G_BEGIN_DECLS

#define OSUX_TYPE_EDITOR_BEATMAP (osux_editor_beatmap_get_type())

G_DECLARE_FINAL_TYPE(OsuxEditorBeatmap, osux_editor_beatmap,
                     OSUX, EDITOR_BEATMAP, GObject);
struct _OsuxEditorBeatmap
{
    GObject parent;
    int error_code;
    char *filepath;
    char *filename;
    char *audio_filepath;

    osux_beatmap beatmap;
    GtkWidget *main_view;

    //Properties
    OsuxAdjustment *AudioLeadIn;
    OsuxAdjustment *BeatDivisor;
    OsuxAdjustment *PreviewTime;
    OsuxAdjustment *StackLeniency;
    OsuxAdjustment *SliderMultiplier;
    OsuxAdjustment *SliderTickRate;
    OsuxAdjustment *TimelineZoom;
    OsuxAdjustment *GridSize;
    OsuxAdjustment *DistanceSpacing;

    OsuxAdjustment *Time; // Position Of The Editor In The Song;

    OsuxAdjustment *ApproachRate;
    OsuxAdjustment *CircleSize;
    OsuxAdjustment *OverallDifficulty;
    OsuxAdjustment *HPDrainRate;

    OsuxAdjustment *BeatmapID;
    OsuxAdjustment *BeatmapSetID;

    GtkTreeStore *Objects;

    GtkTreeIter TimingPoints;
    GtkTreeIter HitObjects;
    GtkTreeIter Bookmarks;
    GtkTreeIter Events;
    GtkTreeIter Colors;
};

OsuxEditorBeatmap *osux_editor_beatmap_new(char const *filepath);
void osux_editor_beatmap_save(OsuxEditorBeatmap *beatmap);

G_END_DECLS

#endif //OSUX_EDITOR_BEATMAP_H
