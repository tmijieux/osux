#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <gtk/gtk.h>
#include "osux/hitobject.h"

typedef struct edosu_color_ {
    double r, g, b, a;
} edosu_color;

void edosu_draw_object(osux_hitobject *ho, cairo_t *cr,
                       int64_t position, edosu_color *cl);

#endif //RENDER_H
