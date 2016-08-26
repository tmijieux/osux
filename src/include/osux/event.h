#ifndef OSUX_EVENT_H
#define OSUX_EVENT_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib/gi18n.h>

#define OSUX_EVENT_HEADER_

typedef struct osux_event_ osux_event;
typedef struct osux_event_object_ osux_event_object;
typedef struct osux_event_command_ osux_event_command;

// arg 3 is boolean to whether display the number or the string
// when printing to '.osu'

#define EVENT_OBJECTS(OBJECT)                                           \
    OBJECT(0, N_("BackgroundImage"), 0, BACKGROUND_IMAGE, parse_bg_image_object) \
    OBJECT(1, N_("Video"),           1, VIDEO, parse_video_object)      \
    OBJECT(2, N_("BreakPeriod"),     0, BREAK, parse_break_period_object) \
    OBJECT(3, N_("BackgroundColour"),0, BACKGROUND_COLOUR, parse_bg_colour_object) \
    OBJECT(4, N_("Sprite"),          1,  SPRITE, parse_sprite_object)   \
    OBJECT(5, N_("Sample"),          1, SAMPLE, parse_sample_object)    \
    OBJECT(6, N_("Animation"),       1,  ANIMATION, parse_animation_object) \

#define EVENT_COMMANDS(COMMAND)                                         \
    COMMAND(FADE, N_("Fade"), F, parse_fade_cmd)                        \
    COMMAND(MOVE, N_("Move"), M, parse_move_cmd)                        \
    COMMAND(MOVE_X, N_("MoveX"), MX, parse_movex_cmd)                   \
    COMMAND(MOVE_Y, N_("MoveY"), MY, parse_movey_cmd)                   \
    COMMAND(SCALE, N_("Scale"), S, parse_scale_cmd)                     \
    COMMAND(VECTOR_SCALE, N_("VectorScale"), V, parse_vscale_cmd)       \
    COMMAND(ROTATE, N_("Rotate"), R, parse_rotate_cmd)                  \
    COMMAND(COLOR, N_("Color"), C, parse_color_cmd)                     \
    COMMAND(PARAMETER, N_("Parameter"), P, parse_parameter_cmd)         \
    COMMAND(LOOP, N_("Loop"), L, parse_loop_cmd)                        \
    COMMAND(TRIGGER, N_("Trigger"), T, parse_trigger_cmd)               \

#define EVENT_LAYERS(LAYER)                     \
    LAYER(0,  N_("Background"), BACKGROUND)     \
    LAYER(1,  N_("Fail"),       FAIL)           \
    LAYER(2,  N_("Pass"),       PASS)           \
    LAYER(3,  N_("Foreground"), FOREGROUND)     \

#define EVENT_LOOPS(LOOP)                       \
    LOOP(0, N_("LoopOnce"),    LOOP_ONCE)       \
    LOOP(1, N_("LoopForever"), LOOP_FOREVER)    \

#define EVENT_ORIGINS(ORIG)                     \
    ORIG(0, N_("TopLeft"),      TOP_LEFT)       \
    ORIG(1, N_("TopCentre"),    TOP_CENTRE)     \
    ORIG(2, N_("TopRight"),     TOP_RIGHT)      \
    ORIG(3, N_("CentreLeft"),   CENTRE_LEFT)    \
    ORIG(4, N_("Centre"),       CENTRE)         \
    ORIG(5, N_("CentreRight"),  CENTRE_RIGHT)   \
    ORIG(6, N_("BottomLeft"),   BOTTOM_LEFT)    \
    ORIG(7, N_("BottomCentre"), BOTTOM_CENTRE)  \
    ORIG(8, N_("BottomRight"),  BOTTOM_RIGHT)   \

#include "osux/event_enum.h"

struct osux_event_object_ {
    char *filename;
    int layer;
    int origin;
    int x; int y;

    int sample_volume;
    int anim_loop;
    int anim_frame_count;
    int anim_frame_delay;
    int r, g, b;
};
struct osux_event_command_ {
    int easing;

    double o1, o2;   // opacity
    double s1, s2;   // scale
    double sx1, sx2;   // scale x
    double sy1, sy2;   // scale y
    double a1, a2; // radian

    int x1; int y1;
    int x2; int y2;
    int r1, g1, b1;
    int r2, g2, b2;
    char param;

    int loop_count;
    char *trigger;

    struct osux_event_command_ *next;
};

struct osux_event_ {
    int type;
    uint32_t level;
    int offset;
    int end_offset;

    osux_event_object object;
    osux_event_command command;

    uint32_t osu_version;
    osux_event *parent;
    uint32_t child_count;
    uint32_t child_bufsize;
    osux_event **childs;
};


#ifdef __cplusplus
extern "C" {
#endif

int osux_event_init(osux_event *event, char *line, uint32_t osu_version);
int osux_event_free(osux_event *event);
int osux_event_build_tree(osux_event *event);
int osux_event_prepare(osux_event *ev);
void osux_event_print(osux_event *ev, FILE *f);

void osux_event_move(osux_event *from, osux_event *to);
void osux_event_copy(osux_event *from, osux_event *to);
char const *osux_event_detail_string(osux_event *ev);

#define EVENT_IS_OBJECT(ev)  (!((ev)->level))
#define EVENT_IS_COMMAND(ev)  (!!((ev)->level))
#define EVENT_TYPE_IS_COMPOUND(type)                                    \
    ((type) == EVENT_COMMAND_TRIGGER || (type) == EVENT_COMMAND_LOOP)


#include "event_string.h"

#ifdef __cplusplus
}
#endif

#undef OSUX_EVENT_HEADER_
#endif // OSUX_EVENT_H
