#ifndef OSUX_EDITOR_BEATMAP_H
#define OSUX_EDITOR_BEATMAP_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define OSUX_TYPE_EDITOR_BEATMAP (osux_editor_beatmap_get_type())

G_DECLARE_FINAL_TYPE(OsuxEditorBeatmap, osux_editor_beatmap,
                     OSUX, EDITOR_BEATMAP, GObject);

OsuxEditorBeatmap *osux_editor_beatmap_new(char const *filename);

G_END_DECLS

#endif //OSUX_EDITOR_BEATMAP_H
