#include <glib.h>
#include <glib/gi18n.h>
#include <string.h>

#include "osux/event.h"
#include "osux/error.h"
#include "osux/util.h"

#define OBJECT_TO_STRING(id_, pretty_, capital_)        \
    [id_] = pretty_,
#define COMMAND_TO_STRING(capital_, pretty_, str_id_)   \
    [EVENT_COMMAND_##capital_] = pretty_,

static char const *event_str[MAX_EVENT_TYPE] = {
    EVENT_OBJECTS(OBJECT_TO_STRING)
    EVENT_COMMANDS(COMMAND_TO_STRING)
};

char const *osux_event_type_get_name(int event_type)
{
    if (event_type < 0 || event_type >= MAX_EVENT_TYPE)
        return NULL;
    return gettext(event_str[event_type]);
}

static int add_child(osux_event *parent, osux_event *child)
{
    g_assert(parent != NULL);
    g_assert(child != NULL);
    g_assert(parent->level+1 == child->level);
    g_assert(parent->level == 0 || // object ...
             parent->type == EVENT_COMMAND_LOOP || // ... or special commands
             parent->type == EVENT_COMMAND_TRIGGER);
    HANDLE_ARRAY_SIZE(parent->childs, parent->child_count, parent->child_bufsize);
    parent->childs[parent->child_count++] = child;
    return 0;
}

// private macro; parse_top_level_event only;
#define MATCH_OBJECT__(id, pretty_, capital_)                   \
    do {                                                        \
        if (!strcmp(type, pretty_) || !strcmp(type, #id)) {     \
            event->type = EVENT_OBJECT_##capital_;              \
            return 0;                                           \
        }                                                       \
    } while (0);

static int parse_object_event(osux_event *event, char **line_split, unsigned size)
{
    if (!size)
        return -OSUX_ERR_INVALID_EVENT;
    event->type = -1;
    char *type = line_split[0];
    EVENT_OBJECTS(MATCH_OBJECT__);
    return -OSUX_ERR_INVALID_EVENT_OBJECT;
}

#define MATCH_COMMAND__(capital_, pretty_, str_value_)  \
    do {                                                \
        if (!strcmp(type, #str_value_))  {              \
            event->type = EVENT_COMMAND_##capital_;     \
            return 0;                                   \
        }                                               \
    } while (0);

static int parse_command_event(osux_event *event, char **line_split, unsigned size)
{
    if (!size)
        return -OSUX_ERR_INVALID_EVENT;
    event->type = -1;
    char *type = line_split[0];
    EVENT_COMMANDS(MATCH_COMMAND__);
    return -OSUX_ERR_INVALID_EVENT_COMMAND;
}

int osux_event_init(osux_event *event, char *line, uint32_t osu_version)
{
    int err = 0;
    memset(event, 0, sizeof *event);
    char *c = line;
    while (*c && (*c == '_' || *c == ' '))
    {
        *c = ' '; // replace by space; the string needs to be trimmed
        ++ event->level;
        ++ c;
    }
    event->osu_version = osu_version;

    g_strstrip(line);
    char **split = g_strsplit(line, ",", 0);
    unsigned size = strsplit_size(split);
    if (!event->level)
        err = parse_object_event(event, split, size);
    else
        err = parse_command_event(event, split, size);
    g_strfreev(split);
    return err;
}

int osux_event_free(osux_event *event)
{
    (void) event;
    return 0;
}

bool osux_event_is_object(osux_event *event)
{
    return !event->level;
}

bool osux_event_is_command(osux_event *event)
{
    return !!event->level;
}

#define EVENT_MAX_STACK_SIZE 200
static osux_event *event_stack[EVENT_MAX_STACK_SIZE] = { NULL, };

int osux_event_build_tree(osux_event *event)
{
    if (event->level >= EVENT_MAX_STACK_SIZE)
        return -OSUX_ERR_MEMORY_TOO_MUCH_NESTED_EVENT;

    event_stack[event->level] = event;
    if (event->level) {
        osux_event *parent = event_stack[event->level-1];
        return add_child(parent, event);
    }
    return 0;
}

