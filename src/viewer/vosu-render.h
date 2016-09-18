#ifndef VOSU_RENDER_H
#define VOSU_RENDER_H

#include <stdint.h>
#include <gtk/gtk.h>

typedef struct vosu_renderer_ vosu_renderer;

#include "osux/hitobject.h"
#include "osux/replay.h"

G_BEGIN_DECLS

struct vosu_renderer_ {
    cairo_t *cr;
    int64_t game_mode;
    int64_t position;
    double approach_rate;
    int approach_time;
    double base_circle_size;
    int circle_size;
    double r, g, b, a;
};

void vosu_draw_object_std(vosu_renderer *r, osux_hitobject *ho);
void vosu_draw_object_taiko(vosu_renderer const *r, osux_hitobject *ho);

void vosu_draw_object(vosu_renderer *r, osux_hitobject *ho);
void vosu_draw_cursor(vosu_renderer const *r, osux_replay_data *cursor);

void vosu_draw_playfield_taiko(vosu_renderer const *r);
void vosu_draw_playfield(vosu_renderer *r);

G_END_DECLS

#endif // VOSU_RENDER_H
