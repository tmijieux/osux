#ifndef VOSU_VIEW_H
#define VOSU_VIEW_H

#include <gtk/gtk.h>
#include <stdint.h>
#include <stdbool.h>

G_BEGIN_DECLS

#define VOSU_TYPE_VIEW (vosu_view_get_type())

G_DECLARE_FINAL_TYPE(VosuView, vosu_view, VOSU, VIEW, GtkBox);

struct _VosuView
{
    GtkBox parent;

    GtkDrawingArea *drawing_area;
    GtkAdjustment *time_adjust;
    GtkRange *time_range;
    gdouble time_max;
    GSequence *hitobjects;

    GtkButton *play_button;
    GtkButton *pause_button;

    int64_t position;
    int64_t start_position;

    gboolean playing;
    guint tick_id;
    gint64 first_frame_time;

    gint color_id;
};

VosuView *vosu_view_new(void);
void vosu_view_set_max_time(VosuView *view, uint64_t max_time, double page_range);
void vosu_view_set_hit_objects(VosuView *view, GSequence *hitobjects);

G_END_DECLS

#endif //VOSU_VIEW_H
