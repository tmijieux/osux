#ifndef EDOSU_VIEW_H
#define EDOSU_VIEW_H

#include <gtk/gtk.h>
#include <stdint.h>
#include <stdbool.h>

G_BEGIN_DECLS

#define EDOSU_TYPE_VIEW (edosu_view_get_type())

G_DECLARE_FINAL_TYPE(EdosuView, edosu_view, EDOSU, VIEW, GtkBox);

struct _EdosuView
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

EdosuView *edosu_view_new(void);
void edosu_view_set_max_time(EdosuView *view, uint64_t max_time, double page_range);
void edosu_view_set_hit_objects(EdosuView *view, GSequence *hitobjects);

G_END_DECLS

#endif //EDOSU_VIEW_H
