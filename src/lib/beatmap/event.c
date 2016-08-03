#include <string.h>
#include "osux/event.h"

int osux_event_init(osux_event *event, char *line, uint32_t osu_version)
{
    memset(event, 0, sizeof *event);
}

int osux_event_free(osux_event *event)
{
    (void) event;
}
