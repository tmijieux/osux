#ifndef EDOSU_APPLICATION_H
#define EDOSU_APPLICATION_H

#include <gtk/gtk.h>
#include "edosu-beatmap.h"

G_BEGIN_DECLS

#define EDOSU_TYPE_APPLICATION (edosu_application_get_type ())

G_DECLARE_FINAL_TYPE(EdosuApplication, edosu_application,
                     EDOSU, APPLICATION, GtkApplication)

EdosuApplication *edosu_application_new(void);
void edosu_application_set_current_beatmap(EdosuApplication *app,
                                           EdosuBeatmap *bm);
EdosuBeatmap *
edosu_application_get_beatmap_by_view(EdosuApplication *app, EdosuView *view);
EdosuBeatmap *
edosu_application_get_beatmap_by_path(EdosuApplication *app, gchar const *path);
void edosu_application_close_beatmap(EdosuApplication *app, EdosuBeatmap *beatmap);

G_END_DECLS

#endif // EDOSU_APPLICATION_H
