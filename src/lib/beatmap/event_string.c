#include "osux/event.h"

#define OBJECT_TO_STRING(id_, pretty_, display_, capital_, parser)      \
    [EVENT_OBJECT_##capital_] = pretty_,
#define COMMAND_TO_STRING(capital_, pretty_, str_id_, parser)   \
    [EVENT_COMMAND_##capital_] = pretty_,

#define OBJECT_TO_MSTRING(id_, pretty_, display_, capital_, parser)     \
    [EVENT_OBJECT_##capital_] = (display_ ? pretty_ : #id_),
#define COMMAND_TO_MSTRING(capital_, pretty_, str_id_, parser)  \
    [EVENT_COMMAND_##capital_] = #str_id_,

#define LAYER_TO_STRING(id_, pretty_, capital_) \
    [EVENT_LAYER_##capital_] = pretty_,
#define LOOP_TO_STRING(id_, pretty_, capital_)  \
    [EVENT_##capital_] = pretty_,
#define ORIG_TO_STRING(id_, pretty_, capital_)  \
    [EVENT_ORIGIN_##capital_] = pretty_,


static char const *event_str[MAX_EVENT_TYPE] = {
    EVENT_OBJECTS(OBJECT_TO_STRING)
    EVENT_COMMANDS(COMMAND_TO_STRING)
};
static char const *event_map_str[MAX_EVENT_TYPE] = {
    EVENT_OBJECTS(OBJECT_TO_MSTRING)
    EVENT_COMMANDS(COMMAND_TO_MSTRING)
};

static char const *layer_str[MAX_LAYER] = {
    EVENT_LAYERS(LAYER_TO_STRING)
};
static char const *loop_str[MAX_LOOP] = {
    EVENT_LOOPS(LOOP_TO_STRING)
};
static char const *orig_str[MAX_ORIGIN] = {
    EVENT_ORIGINS(ORIG_TO_STRING)
};

char const *osux_event_type_get_name(int event_type)
{
    if (event_type < 0 || event_type >= MAX_EVENT_TYPE)
        return NULL;
    return event_str[event_type];
}

char const *osux_event_type_get_mname(int event_type)
{
    if (event_type < 0 || event_type >= MAX_EVENT_TYPE)
        return NULL;
    return event_map_str[event_type];
}


char const *osux_event_layer_get_name(int layer)
{
    if (layer < 0 || layer >= MAX_LAYER)
        return NULL;
    return layer_str[layer];
}

char const *osux_event_loop_get_name(int loop)
{
    if (loop < 0 || loop >= MAX_LOOP)
        return NULL;
    return loop_str[loop];
}

char const *osux_event_origin_get_name(int orig)
{
    if (orig < 0 || orig >= MAX_ORIGIN)
        return NULL;
    return orig_str[orig];
}
