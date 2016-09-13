#ifndef VOSU_VIEW_H
#define VOSU_VIEW_H

#include <gtk/gtk.h>
#include <stdint.h>
#include <stdbool.h>
#include "vosu-player.h"

G_BEGIN_DECLS

#define VOSU_TYPE_VIEW (vosu_view_get_type())

G_DECLARE_FINAL_TYPE(VosuView, vosu_view, VOSU, VIEW, GtkBox);

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
    int64_t start_position;
    gboolean playing;
    guint tick_id;
    gint64 first_frame_time;

    GSequence *hitobjects;
    GSequence *cursor_data;
    int approach_time;

    VosuPlayer *player; /* music player */
};

VosuView *vosu_view_new(void);
void vosu_view_set_properties(VosuView *view,
                              uint64_t max_time,
                              double page_range,
                              GSequence *hitobjects,
                              double approach_rate,
                              int mods, gchar const *music_file);

G_END_DECLS

#endif //VOSU_VIEW_H
