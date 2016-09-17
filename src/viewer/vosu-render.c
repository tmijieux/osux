#include "osux.h"
#include "vosu-render.h"

static void
draw_unsupported_message(vosu_renderer *r)
{
    cairo_t *cr = r->cr;
    cairo_font_extents_t fe;
    cairo_text_extents_t te;
    char const *text;
    text = _("This game mode is currently unsupported, sorry.");

    cairo_set_font_size(cr, 20);
    cairo_font_extents(cr, &fe);
    cairo_set_source_rgb(cr, 0., 0., 0.);
    cairo_text_extents(cr, text, &te);

    double x = 256 - te.x_bearing - te.width / 2;
    double y = 192 - te.y_bearing - te.height / 2;
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text);
}

void vosu_draw_object(vosu_renderer *r, osux_hitobject *ho)
{
    switch(r->game_mode) {
    case GAME_MODE_STD:   vosu_draw_object_std(r, ho);   break;
    case GAME_MODE_TAIKO: vosu_draw_object_taiko(r, ho); break;
    default:
        break;
    }
}

void vosu_draw_playfield(vosu_renderer *r)
{
    switch(r->game_mode) {
    case GAME_MODE_STD: /*nathing*/; break;
    case GAME_MODE_TAIKO: vosu_draw_playfield_taiko(r); break;
    default:     draw_unsupported_message(r);     break;
    }
}
