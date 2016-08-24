#ifndef EDOSU_PROPERTIES_H
#define EDOSU_PROPERTIES_H

#include <gtk/gtk.h>
#include "osux.h"

G_BEGIN_DECLS

#define EDOSU_TYPE_PROPERTIES (edosu_properties_get_type())
G_DECLARE_FINAL_TYPE(EdosuProperties, edosu_properties, EDOSU, PROPERTIES, GtkNotebook);

struct _EdosuProperties
{
    GtkNotebook parent;

    // general
    GtkFileChooser *audio_file_chooser;
    GtkComboBoxText *Mode;
    GtkComboBoxText *SampleSet;
    GtkAdjustment *StackLeniency;
    GtkAdjustment *PreviewTime;
    GtkAdjustment *AudioLeadIn;
    GtkSwitch *Countdown;
    GtkSwitch *LetterboxInBreaks;
    GtkSwitch *WidescreenStoryboard;

    // metadata
    GtkEntry *TitleUnicode;
    GtkEntry *Title;
    GtkEntry *ArtistUnicode;
    GtkEntry *Artist;
    GtkEntry *Creator;
    GtkEntry *Version;
    GtkEntry *Source;
    GtkEntry *Tags;
    GtkAdjustment *BeatmapID;
    GtkAdjustment *BeatmapSetID;

    // editor
    GtkAdjustment *DistanceSpacing;
    GtkAdjustment *BeatDivisor;
    GtkAdjustment *GridSize;
    GtkAdjustment *TimelineZoom;

    // difficulty
    GtkAdjustment *HPDrainRate;
    GtkAdjustment *CircleSize;
    GtkAdjustment *OverallDifficulty;
    GtkAdjustment *ApproachRate;
    GtkAdjustment *SliderMultiplier;
    GtkAdjustment *SliderTickRate;
};

EdosuProperties *edosu_properties_new(void);
void
edosu_properties_load_from_beatmap(EdosuProperties *props, osux_beatmap *beatmap);

G_END_DECLS

#endif //EDOSU_PROPERTIES_H
