#ifndef OSUX_EVENT_H
#define OSUX_EVENT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct osux_event_ osux_event;

#define EVENT_OBJECTS(OBJECT)                           \
    OBJECT(0, BackgroundImage, BACKGROUND_IMAGE)        \
    OBJECT(1, Video, VIDEO)                             \
    OBJECT(2, BreakPeriod, BREAK)                       \
    OBJECT(3, BackgroundColour, BACKGROUND_COLOUR)      \
    OBJECT(4, Sprite, SPRITE)                           \
    OBJECT(5, Sample, SAMPLE)                           \
    OBJECT(6, Animation, ANIMATION)                     \

#define EVENT_COMMANDS(COMMAND)                 \
    COMMAND(FADE, Fade, F)                      \
    COMMAND(MOVE, Move, M)                      \
    COMMAND(MOVE_X, MoveX, MX)                  \
    COMMAND(MOVE_Y, MoveY, MY)                  \
    COMMAND(SCALE, Scale, S)                    \
    COMMAND(VECTOR_SCALE, VectorScale, V)       \
    COMMAND(ROTATE, Rotate, R)                  \
    COMMAND(COLOR, Color, C)                    \
    COMMAND(PARAMETER, Parameter, P)            \
    COMMAND(LOOP, Loop, L)                      \
    COMMAND(TRIGGER, Trigger, T)                \

#define EVENT_LAYERS(LAYER)                     \
    LAYER(0,  Background, BACKGROUND)           \
    LAYER(1,  Fail,       FAIL)                 \
    LAYER(2,  Pass,       PASS)                 \
    LAYER(3,  Foreground, FOREGROUND)           \

#define OBJECT_TO_ENUM_WITH_VALUE(value_, pretty_, capital_)    \
    EVENT_OBJECT_##capital_ = value_,
#define LAYER_TO_ENUM_WITH_VALUE(value_, pretty_, capital_)     \
    EVENT_LAYER_##capital_ = value_,
#define COMMAND_TO_ENUM(capital_, pretty_, string_value)        \
    EVENT_COMMAND_##capital_,

enum event_type {
    EVENT_OBJECTS(OBJECT_TO_ENUM_WITH_VALUE)
    EVENT_COMMANDS(COMMAND_TO_ENUM)
    MAX_EVENT_TYPE,
};
enum event_layer {
    EVENT_LAYERS(LAYER_TO_ENUM_WITH_VALUE)
};

struct osux_event_ {
    int type;
    int layer;
    int64_t offset;
    uint32_t level;
    uint32_t osu_version;
    osux_event *parent;
    uint32_t child_count;
    uint32_t child_bufsize;
    osux_event **childs;


    uint8_t _reserved[30];
};

int osux_event_init(osux_event *event, char *line, uint32_t osu_version);
bool osux_event_is_object(osux_event *event);
bool osux_event_is_command(osux_event *event);
int osux_event_free(osux_event *event);
char const *osux_event_type_get_name(int event_type);
int osux_event_build_tree(osux_event *event);

#endif // OSUX_EVENT_H
