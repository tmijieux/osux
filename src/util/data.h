#ifndef DATA_H
#define DATA_H

FILE *osux_open_config(const char *path, const char *mode);
FILE *osux_open_resource(const char *path, const char *mode);
char *osux_prefix_path(const char *prefix, const char *path);

#endif //DATA_H
