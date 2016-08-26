#ifndef OSUX_EVENT_STRING_H
#define OSUX_EVENT_STRING_H

#ifndef OSUX_EVENT_HEADER_
#error "Never include this file directly"
#endif

char const *osux_event_type_get_name(int event_type);
char const *osux_event_type_get_mname(int event_type);
char const *osux_event_layer_get_name(int layer);
char const *osux_event_loop_get_name(int loop);
char const *osux_event_origin_get_name(int origin);

#endif // OSUX_EVENT_H
