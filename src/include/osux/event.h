#ifndef OSUX_EVENT_H
#define OSUX_EVENT_H

#include <stdint.h>

typedef struct osux_event {
    uint8_t _reserved[30];
} osux_event;

int osux_event_init(osux_event *event, char *line, uint32_t osu_version);
int osux_event_free(osux_event *event);

#endif // OSUX_EVENT_H
