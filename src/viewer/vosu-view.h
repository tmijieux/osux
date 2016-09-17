#ifndef VOSU_VIEW_H
#define VOSU_VIEW_H

#include <gtk/gtk.h>
#include <stdint.h>
#include <stdbool.h>

G_BEGIN_DECLS

#define VOSU_TYPE_VIEW (vosu_view_get_type())

G_DECLARE_FINAL_TYPE(VosuView, vosu_view, VOSU, VIEW, GtkBox);

#include "vosu-player.h"
#include "vosu-render.h"
#include "vosu-sequence.h"

struct _VosuView
{
    GtkBox parent;

    GtkDrawingArea *drawing_area;
    GtkButton *play_button;
    GtkButton *pause_button;
    gdouble time_max;
    GtkRange *time_range;
    GtkAdjustment *time_adjust;

    // animation control
    int64_t position;
    gboolean playing;
    guint tick_id;

    VosuSequence *hitobjects;
    VosuSequence *hitobjects_mod;
    VosuSequence *cursor_data;

    vosu_renderer renderer;
    VosuPlayer *player; /* music player */

    // last time info were displayed in console
    int64_t info_display_old_time;
};

VosuView *vosu_view_new(void);
void vosu_view_set_beatmap_properties(
    VosuView *self,
    uint64_t max_time,    double page_range,
    int64_t game_mode,    VosuSequence *hitobjects,
    double approach_rate, double circle_size,
    gchar const *music_file);

void vosu_view_set_replay_properties(
    VosuView *view,
    VosuSequence *cursor_data, int mods);

G_END_DECLS

#endif //VOSU_VIEW_H
