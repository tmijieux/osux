#ifndef EDOSU_VIEW_H
#define EDOSU_VIEW_H

#include <gtk/gtk.h>
#include <stdint.h>
#include "edosu-gl.h"

G_BEGIN_DECLS

#define EDOSU_TYPE_VIEW (edosu_view_get_type())

G_DECLARE_FINAL_TYPE(EdosuView, edosu_view, EDOSU, VIEW, GtkBox);

struct _EdosuView
{
    GtkBox parent;
    EdosuGL *gl_area;

    GtkAdjustment *time_adjust;
    GtkRange *time_range;
    gdouble time_max;
};

EdosuView *edosu_view_new(void);
void edosu_view_set_max_time(EdosuView *view, uint64_t max_time);
void edosu_view_set_hit_objects(EdosuView *view, GSequence *hitobjects);

G_END_DECLS

#endif //EDOSU_VIEW_H
