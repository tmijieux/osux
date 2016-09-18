#ifndef OSUX_DATA_H
#define OSUX_DATA_H

#include <glib.h>

G_BEGIN_DECLS

FILE *osux_open_config(char const *path, char const *mode);
FILE *osux_open_resource(char const *path, char const *mode);
char *osux_prefix_path(char const *prefix, char const *path);
void osux_set_song_path(char const *path);
char const *osux_get_song_path(void);

G_END_DECLS

#endif // OSUX_DATA_H
