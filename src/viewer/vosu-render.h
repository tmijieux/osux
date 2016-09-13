#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <gtk/gtk.h>
#include "osux/hitobject.h"
#include "osux/replay.h"

typedef struct vosu_color_ {
    double r, g, b, a;
} vosu_color;

void vosu_draw_object(osux_hitobject *ho, cairo_t *cr,
                      int64_t position, vosu_color *cl,
                      int approach_time);
void vosu_draw_cursor(cairo_t *cr, osux_replay_data *cursor);

#endif //RENDER_H
