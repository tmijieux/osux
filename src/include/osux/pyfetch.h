#ifndef OSUX_PYFETCH_H
#define OSUX_PYFETCH_H

#include <glib.h>
#include "osux/beatmap.h"

G_BEGIN_DECLS

osux_beatmap *osux_py_parse_beatmap(const char *filename);

G_END_DECLS

#endif // OSUX_PYFETCH_H
