#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <gtk/gtk.h>
#include "osux/hitobject.h"

typedef struct vosu_color_ {
    double r, g, b, a;
} vosu_color;

void vosu_draw_object(osux_hitobject *ho, cairo_t *cr,
                       int64_t position, vosu_color *cl);

#endif //RENDER_H
