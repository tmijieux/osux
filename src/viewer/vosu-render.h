#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <gtk/gtk.h>
#include "osux/hitobject.h"
#include "osux/replay.h"

typedef struct vosu_renderer_ {
    cairo_t *cr;
    int64_t position;
    int approach_time;
    int circle_size;
    double r, g, b, a;
} vosu_renderer;

void vosu_draw_object(vosu_renderer *r, osux_hitobject *ho);
void vosu_draw_cursor(vosu_renderer const *r, osux_replay_data *cursor);

#endif //RENDER_H
