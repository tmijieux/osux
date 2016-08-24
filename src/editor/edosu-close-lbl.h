#ifndef EDOSU_CLOSE_LABEL_H
#define EDOSU_CLOSE_LABEL_H

#include <gtk/gtk.h>
#include "osux/beatmap.h"

G_BEGIN_DECLS

#define EDOSU_TYPE_CLOSE_LABEL (edosu_close_label_get_type())

G_DECLARE_FINAL_TYPE(EdosuCloseLabel, edosu_close_label,
                     EDOSU, CLOSE_LABEL, GtkBox);

GtkWidget *edosu_close_label_new(gchar const *text,
                                 EdosuApplication *app,
                                 EdosuBeatmap *beatmap);
void edosu_close_label_set_text(EdosuCloseLabel *close_label, gchar const *text);

G_END_DECLS

#endif // EDOSU_CLOSE_LABEL_H
