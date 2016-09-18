#ifndef EDOSU_RENDER_H
#define EDOSU_RENDER_H

#include <stdint.h>
#include <gtk/gtk.h>
#include "osux/hitobject.h"

G_BEGIN_DECLS

typedef struct edosu_color_ {
    double r, g, b, a;
} edosu_color;

void edosu_draw_object(osux_hitobject *ho, cairo_t *cr,
                       int64_t position, edosu_color *cl);

G_END_DECLS

#endif // EDOSU_RENDER_H
