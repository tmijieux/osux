#ifndef OSUX_EVENT_ENUM_H
#define OSUX_EVENT_ENUM_H

#include "osux/event.h"

#define OBJECT_TO_ENUM_WITH_VALUE(value_, pretty_, capital_, parser)    \
    EVENT_OBJECT_##capital_ = value_,
#define COMMAND_TO_ENUM(capital_, pretty_, string_value, parser)        \
    EVENT_COMMAND_##capital_,
#define LAYER_TO_ENUM_WITH_VALUE(value_, pretty_, capital_)     \
    EVENT_LAYER_##capital_ = value_,
#define LOOP_TO_ENUM_WITH_VALUE(value_, pretty_, capital_)     \
    EVENT_##capital_ = value_,
#define ORIG_TO_ENUM_WITH_VALUE(value_, pretty_, capital_)     \
    EVENT_ORIGIN_##capital_ = value_,

enum event_type {
    EVENT_OBJECTS(OBJECT_TO_ENUM_WITH_VALUE)
    EVENT_COMMANDS(COMMAND_TO_ENUM)
    MAX_EVENT_TYPE,
};
enum event_layer {
    EVENT_LAYERS(LAYER_TO_ENUM_WITH_VALUE)
    MAX_LAYER,
};
enum event_loop {
    EVENT_LOOPS(LOOP_TO_ENUM_WITH_VALUE)
    MAX_LOOP,
};

enum  event_origin {
    EVENT_ORIGINS(ORIG_TO_ENUM_WITH_VALUE)
    MAX_ORIGIN,
};

#endif // OSUX_EVENT_ENUM_H

