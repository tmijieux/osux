#ifndef OSUX_EDITOR_WINDOW_H
#define OSUX_EDITOR_WINDOW_H

#include <gtk/gtk.h>
#include "app.h"
#include "beatmap.h"

G_BEGIN_DECLS

#define OSUX_TYPE_EDITOR_WINDOW (osux_editor_window_get_type())

G_DECLARE_FINAL_TYPE(OsuxEditorWindow, osux_editor_window,
                     OSUX, EDITOR_WINDOW, GtkApplicationWindow);

struct _OsuxEditorWindow
{
    GtkApplicationWindow parent;

    GtkWidget *popover;
    GtkWidget *new_circle_button;

    OsuxEditorBeatmap *current_beatmap;
    
    // main
    GtkNotebook *main_tab;

    // properties
    // general
    GtkFileChooserButton *AudioFile;
    GtkComboBoxText *GameMode;
    GtkComboBoxText *SampleSet;
    
    GtkSpinButton *StackLeniency;
    GtkSpinButton *PreviewTime;
    GtkSpinButton *AudioLeadIn;
    GtkSwitch *CountDown;
    GtkSwitch *LetterboxInBreaks;
    GtkSwitch *Widescreen;

    // metadata
    GtkEntry *TitleUnicode;
    GtkEntry *Title;
    GtkEntry *ArtistUnicode;
    GtkEntry *Artist;
    GtkEntry *Creator;
    GtkEntry *Version; // diff name
    GtkEntry *Source;
    GtkEntry *Tags;
    GtkSpinButton *BeatmapID;
    GtkSpinButton *BeatmapSetID;

    // editor
    GtkSpinButton *DistanceSpacing;
    GtkSpinButton *BeatDivisor;
    GtkSpinButton *GridSize;
    GtkSpinButton *TimelineZoom;

    // difficulty
    GtkScale *HPDrainRate;
    GtkScale *CircleSize;
    GtkScale *OverallDifficulty;
    GtkScale *ApproachRate;
    GtkSpinButton *SliderMultiplier;
    GtkSpinButton *SliderTickRate;
};

OsuxEditorWindow *osux_editor_window_new(OsuxEditorApp *app);
gint osux_editor_window_add_beatmap_tab(
    OsuxEditorWindow *win, OsuxEditorBeatmap *beatmap);

G_END_DECLS


#endif //OSUX_EDITOR_WINDOW_H
