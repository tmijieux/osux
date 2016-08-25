#include <glib.h>
#include <glib/gi18n.h>
#include <string.h>
#include <limits.h>

#include "osux/event.h"
#include "osux/error.h"
#include "osux/util.h"

typedef int (*command_parser_t)(osux_event_command*,char**,unsigned);

#define PARSER_DECLARE(a, b, c, parser_)                \
    static int parser_(osux_event*,char**,unsigned);

#define CMD_PARSER_DECLARE(a, b, c, parser_)                    \
    static int parser_(osux_event_command*,char**,unsigned);

#define OBJECT_TO_PARSER(id, pr, ca, parser_)   \
    [id] = parser_,
#define COMMAND_TO_PARSER(ca, pr, s_id, parser_)        \
    [EVENT_COMMAND_##ca] = parser_,

#define EVENT_TYPE_IS_COMPOUND(type)                                    \
    ((type) == EVENT_COMMAND_TRIGGER || (type) == EVENT_COMMAND_LOOP)
#define EVENT_IS_COMMAND(ev)  (!!((ev)->level))
#define EVENT_IS_OBJECT(ev)  (!((ev)->level))


static int parse_command(osux_event*,char**,unsigned);
EVENT_OBJECTS(PARSER_DECLARE);
EVENT_COMMANDS(CMD_PARSER_DECLARE);
static command_parser_t command_parsers[] = {
    EVENT_COMMANDS(COMMAND_TO_PARSER)
};

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
    child->parent = parent;
    return 0;
}

// private macro; parse_top_level_event only;
#define MATCH_OBJECT__(id, pretty_, capital_, parser_)          \
    do {                                                        \
        if (!strcmp(type, pretty_) || !strcmp(type, #id)) {     \
            event->type = EVENT_OBJECT_##capital_;              \
            return parser_(event, split, size);                 \
        }                                                       \
    } while (0);


#define MATCH_COMMAND__(capital_, pretty_, str_value_, parser_) \
    do {                                                        \
        if (!strcmp(type, #str_value_))  {                      \
            event->type = EVENT_COMMAND_##capital_;             \
            return parse_command(event, split, size);           \
        }                                                       \
    } while (0);

#define MATCH_LAYER__(id, pretty_, capital_)                    \
    do {                                                        \
        if (!strcmp(layer, pretty_) || !strcmp(layer, #id))     \
            return EVENT_LAYER_##capital_;                      \
    } while (0);

#define MATCH_LOOP__(id, pretty_, capital_)                     \
    do {                                                        \
        if (!strcmp(anim, pretty_) || !strcmp(anim, #id))       \
            return EVENT_##capital_;                            \
    } while (0);

#define MATCH_ORIG__(id, pretty_, capital_)     \
    do {                                        \
        if (!strcmp(orig, pretty_))             \
            return EVENT_ORIGIN_##capital_;     \
    } while (0);

static int parse_object_event(osux_event *event, char **split, unsigned size)
{
    if (!size)
        return -OSUX_ERR_INVALID_EVENT;
    event->type = -1;
    event->offset = INT64_MAX;
    char *type = split[0];
    EVENT_OBJECTS(MATCH_OBJECT__);
    return -OSUX_ERR_INVALID_EVENT_OBJECT;
}
static int parse_command_event(osux_event *event, char **split, unsigned size)
{
    if (!size)
        return -OSUX_ERR_INVALID_EVENT;
    event->type = -1;
    char *type = split[0];
    EVENT_COMMANDS(MATCH_COMMAND__);
    printf("no command matched '%s'\n" , type);
    return -OSUX_ERR_INVALID_EVENT_COMMAND;
}

static int parse_command(osux_event *ev, char **split, unsigned size)
{
    command_parser_t parser;
    if (ev->type < 0 || ev->type >= MAX_EVENT_TYPE)
        return -OSUX_ERR_INVALID_EVENT_COMMAND;
    parser = command_parsers[ev->type];

    if (!EVENT_TYPE_IS_COMPOUND(ev->type)) {
        if (size < 4)
            return -OSUX_ERR_INVALID_EVENT_COMMAND;
        ev->command.easing = atoi(split[1]);
        ev->offset = atoi(split[2]);
        if (!strcmp(split[3], ""))
            ev->end_offset = ev->offset;
        else
            ev->end_offset = atoi(split[3]);
        return parser(&ev->command, split+4, size-4);
    }
    return parser(&ev->command, split, size);
}

static int parse_layer(char const *layer)
{
    EVENT_LAYERS(MATCH_LAYER__);
    return -1;
}

static int parse_loop(char const *anim)
{
    EVENT_LOOPS(MATCH_LOOP__);
    return -1;
}

static int parse_origin(char const *orig)
{
    EVENT_ORIGINS(MATCH_ORIG__);
    return -1;
}

static int parse_bg_image_object(
    osux_event *ev, char **split, unsigned size)
{
    if (size != 3 && size != 5)
        return -OSUX_ERR_INVALID_EVENT_OBJECT;

    ev->offset = atoi(split[1]);
    ev->object.filename = g_strdup(split[2]);
    if (size == 5) {
        ev->object.x = atoi(split[3]);
        ev->object.y = atoi(split[4]);
    }
    return 0;
}

static int parse_video_object(
    osux_event *ev, char **split, unsigned size)
{
    if (size != 3)
        return -OSUX_ERR_INVALID_EVENT_OBJECT;

    ev->object.layer = EVENT_LAYER_BACKGROUND;
    ev->offset = atoi(split[1]);
    ev->object.filename = g_strdup(split[2]);
    return 0;
}

static int parse_break_period_object(
    osux_event *ev, char **split, unsigned size)
{
    if (size != 3)
        return -OSUX_ERR_INVALID_EVENT_OBJECT;
    ev->offset = atoi(split[1]);
    ev->end_offset = atoi(split[1]);
    return 0;
}

static int parse_bg_colour_object(
    osux_event *ev, char **split, unsigned size)
{
    if (size != 5)
        return -OSUX_ERR_INVALID_EVENT_OBJECT;

    ev->offset = atoi(split[1]);
    ev->object.r = atoi(split[2]);
    ev->object.g = atoi(split[3]);
    ev->object.b = atoi(split[4]);

    return 0;
}

static int parse_sprite_object(
    osux_event *ev, char **split, unsigned size)
{
    if (size != 6)
        return -OSUX_ERR_INVALID_EVENT_OBJECT;

    ev->object.layer = parse_layer(split[1]);
    ev->object.origin = parse_origin(split[2]);
    ev->object.filename = g_strdup(split[3]);
    ev->object.x = atoi(split[4]);
    ev->object.y = atoi(split[5]);

    return 0;
}

static int parse_sample_object(
    osux_event *ev, char **split, unsigned size)
{
    if (size < 4)
        return -OSUX_ERR_INVALID_EVENT_OBJECT;
    ev->offset = atoi(split[1]);
    ev->object.layer = parse_layer(split[2]);
    ev->object.filename = g_strdup(split[3]);
    ev->object.sample_volume = 100;
    if (size >= 5)
        ev->object.sample_volume = atoi(split[4]);

    return 0;
}

static int parse_animation_object(
    osux_event *ev, char **split, unsigned size)
{
    if (size < 8)
        return -OSUX_ERR_INVALID_EVENT_OBJECT;
    ev->object.layer = parse_layer(split[1]);
    ev->object.origin = parse_origin(split[2]);
    ev->object.filename = g_strdup(split[3]);
    ev->object.x = atoi(split[4]);
    ev->object.y = atoi(split[5]);
    ev->object.anim_frame_count = atoi(split[6]);
    ev->object.anim_frame_delay = atoi(split[7]);
    ev->object.anim_loop = EVENT_LOOP_FOREVER;
    if (size == 9)
        ev->object.anim_loop = parse_loop(split[8]);
    return 0;
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
    g_free(event->childs);
    g_free(event->object.filename);
    g_free(event->command.trigger);

    osux_event_command *ptr = &event->command;
    osux_event_command *next = ptr->next;
    while (next) {
        ptr = next;
        next = next->next;
        g_free(ptr);
    }

    memset(event, 0, sizeof*event);
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

static int parse_fade_cmd(osux_event_command *cmd, char **split, unsigned size)
{
    if (size == 1)
        cmd->o1 = cmd->o2 = g_ascii_strtod(split[0], NULL);
    else {
        cmd->o1 = g_ascii_strtod(split[0], NULL);
        cmd->o2 = g_ascii_strtod(split[1], NULL);
    }
    if (size > 2) {
        cmd->next = g_malloc0(sizeof*cmd->next);
        cmd->next->easing = cmd->easing;
        parse_fade_cmd(cmd->next, split+1, size-1);
    }
    return 0;
}

static int parse_move_cmd(osux_event_command *cmd, char **split, unsigned size)
{
    if (size < 2)
        return -OSUX_ERR_INVALID_EVENT_COMMAND;
    cmd->x1 = atoi(split[0]);
    cmd->y1 = atoi(split[1]);

    if (size == 2) {
        cmd->x2 = cmd->x1;
        cmd->y2 = cmd->y1;
        return 0;
    }
    if (size < 4)
        return -OSUX_ERR_INVALID_EVENT_COMMAND;

    cmd->x2 = atoi(split[2]);
    cmd->y2 = atoi(split[3]);

    if (size > 4) {
        cmd->next = g_malloc0(sizeof*cmd->next);
        cmd->next->easing = cmd->easing;
        return parse_move_cmd(cmd->next, split+2, size-2);
    }
    return 0;
}

static int parse_movex_cmd(osux_event_command *cmd, char **split, unsigned size)
{
    if (size == 1)
        cmd->x1 = cmd->x2 = atoi(split[0]);
    else {
        cmd->x1 = atoi(split[0]);
        cmd->x2 = atoi(split[1]);
    }
    if (size > 2) {
        cmd->next = g_malloc0(sizeof*cmd->next);
        cmd->next->easing = cmd->easing;
        parse_movex_cmd(cmd->next, split+1, size-1);
    }
    return 0;
}

static int parse_movey_cmd(osux_event_command *cmd, char **split, unsigned size)
{
    if (size == 1)
        cmd->y1 = cmd->y2 = atoi(split[0]);
    else {
        cmd->y1 = atoi(split[0]);
        cmd->y2 = atoi(split[1]);
    }
    if (size > 2) {
        cmd->next = g_malloc0(sizeof*cmd->next);
        cmd->next->easing = cmd->easing;
        parse_movey_cmd(cmd->next, split+1, size-1);
    }
    return 0;
}

static int parse_scale_cmd(osux_event_command *cmd, char **split, unsigned size)
{
    if (size == 1)
        cmd->s1 = cmd->s2 = g_ascii_strtod(split[0], NULL);
    else {
        cmd->s1 = g_ascii_strtod(split[0], NULL);
        cmd->s2 = g_ascii_strtod(split[1], NULL);
    }
    if (size > 2) {
        cmd->next = g_malloc0(sizeof*cmd->next);
        cmd->next->easing = cmd->easing;
        parse_scale_cmd(cmd->next, split+1, size-1);
    }
    return 0;
}

static int parse_vscale_cmd(osux_event_command *cmd, char **split, unsigned size)
{
    if (size < 2)
        return -OSUX_ERR_INVALID_EVENT_COMMAND;
    cmd->sx1 = g_ascii_strtod(split[0], NULL);
    cmd->sy1 = g_ascii_strtod(split[1], NULL);

    if (size == 2) {
        cmd->sx2 = cmd->sx1;
        cmd->sy2 = cmd->sy1;
        return 0;
    }
    if (size < 4)
        return -OSUX_ERR_INVALID_EVENT_COMMAND;

    cmd->sx2 = g_ascii_strtod(split[2], NULL);
    cmd->sy2 = g_ascii_strtod(split[3], NULL);

    if (size > 4) {
        cmd->next = g_malloc0(sizeof*cmd->next);
        cmd->next->easing = cmd->easing;
        return parse_vscale_cmd(cmd->next, split+2, size-2);
    }
    return 0;
}

static int parse_rotate_cmd(osux_event_command *cmd, char **split, unsigned size)
{
    if (size == 1)
        cmd->a1 = cmd->a2 = g_ascii_strtod(split[0], NULL);
    else {
        cmd->a1 = g_ascii_strtod(split[0], NULL);
        cmd->a2 = g_ascii_strtod(split[1], NULL);
    }
    if (size > 2) {
        cmd->next = g_malloc0(sizeof*cmd->next);
        cmd->next->easing = cmd->easing;
        parse_scale_cmd(cmd->next, split+1, size-1);
    }
    return 0;
}

static int parse_color_cmd(osux_event_command *cmd, char **split, unsigned size)
{
    if (size < 3)
        return -OSUX_ERR_INVALID_EVENT_COMMAND;
    cmd->r1 = atoi(split[0]);
    cmd->g1 = atoi(split[1]);
    cmd->b1 = atoi(split[2]);

    if (size == 3) {
        cmd->r2 = cmd->r1;
        cmd->g2 = cmd->g1;
        cmd->b2 = cmd->b1;
        return 0;
    }
    if (size < 6)
        return -OSUX_ERR_INVALID_EVENT_COMMAND;

    cmd->r2 = atoi(split[3]);
    cmd->g2 = atoi(split[4]);
    cmd->b2 = atoi(split[5]);

    if (size > 6) {
        cmd->next = g_malloc0(sizeof*cmd->next);
        cmd->next->easing = cmd->easing;
        return parse_color_cmd(cmd->next, split+3, size-3);
    }
    return 0;
}


static int parse_parameter_cmd(osux_event_command *cmd, char **split, unsigned size)
{
    if (size != 1)
        return -OSUX_ERR_INVALID_EVENT_PARAMETER_COMMAND;
    cmd->param = split[0][0];
    return 0;
}


/*
  COMPOUND (compound command does not have the default arguments
  (easing,offset,end_offset) like the other commands!!!
*/

/*
  loop end offset is either loop->offset + max(end_offset(childs) if LOOP_ONCE
  or parent->end_offset if LOOP_FOREVER

  !! warning:
  when LOOP_ONCE, parent end_offset is computed from loop end_offset; BUT...
  when LOOP_FOREVER loop end_offset is computed from parent end_offset;
*/
static int parse_loop_cmd(osux_event_command *cmd, char **split, unsigned size)
{
    if (size != 3)
        return -OSUX_ERR_INVALID_EVENT_LOOP_COMMAND;
    osux_event *ev = NULL;
    ev = (osux_event*) (((char*)cmd) - OFFSET_OF(osux_event, command));

    ev->offset = atoi(split[1]);
    cmd->loop_count = atoi(split[2]);
    return 0;
}

static int parse_trigger_cmd(osux_event_command *cmd, char **split, unsigned size)
{
    if (size != 4)
        return -OSUX_ERR_INVALID_EVENT_TRIGGER_COMMAND;
    osux_event *ev = NULL;
    ev = (osux_event*) (((char*)cmd) - OFFSET_OF(osux_event, command));

    cmd->trigger = g_strdup(split[1]);
    ev->offset = atoi(split[2]);
    ev->end_offset = atoi(split[3]);
    return 0;
}

char const *osux_event_detail_string(osux_event *ev)
{
    switch (ev->type) {
    case EVENT_OBJECT_VIDEO:
    case EVENT_OBJECT_BACKGROUND_IMAGE:
    case EVENT_OBJECT_SPRITE:
    case EVENT_OBJECT_SAMPLE:
        return ev->object.filename;
    case EVENT_COMMAND_TRIGGER:
        return ev->command.trigger;
    default:
        break;
    }
    return "";
}


#define UPDATE(to, from, conv)                  \
    do {  if (to conv from){                    \
            to = from;                          \
        } } while(0)

int osux_event_prepare(osux_event *ev)
{
    if (EVENT_IS_COMMAND(ev) && ev->level == 1) {
        UPDATE(ev->parent->offset, ev->offset, >);
        if (!EVENT_TYPE_IS_COMPOUND(ev->type)) {
            // loop end offset is not computed at this point
            // TODO check for trigger
            UPDATE(ev->parent->end_offset, ev->end_offset, <);
        }
    }
    return 0;
}

void osux_event_move(osux_event *from, osux_event *to)
{
    *to = *from;

    to->parent = NULL;
    g_clear_pointer(&to->childs, g_free);
    to->child_count = 0;
    to->child_bufsize = 0;

    memset(from, 0, sizeof*from);
}

void osux_event_copy(osux_event *from, osux_event *to)
{
    *to = *from;
    to->parent = NULL;
    to->childs = NULL;
    to->child_count = 0;
    to->child_bufsize = 0;

    to->object.filename = g_strdup(from->object.filename);
    to->command.trigger = g_strdup(from->command.trigger);
}
