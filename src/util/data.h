#ifndef DATA_H
#define DATA_H

FILE *osux_open_config(const char *path, const char *mode);
FILE *osux_open_resource(const char *path, const char *mode);
char *osux_prefix_path(const char *prefix, const char *path);
void osux_set_song_path(const char *path);
const char *osux_get_song_path(void);

#endif //DATA_H
