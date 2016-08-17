#ifndef OSUX_EDITOR_BEATMAPLABEL_H
#define OSUX_EDITOR_BEATMAPLABEL_H


#include <gtk/gtk.h>
#include "osux/beatmap.h"

G_BEGIN_DECLS

#define OSUX_TYPE_CLOSE_LABEL (osux_close_label_get_type())

G_DECLARE_FINAL_TYPE(OsuxCloseLabel, osux_close_label,
                     OSUX, CLOSE_LABEL, GtkBox);

struct _OsuxCloseLabel
{
    GtkBox parent;
    GtkLabel *label_value;
};

GtkWidget *osux_close_label_new(gchar const *label_text);

G_END_DECLS

#endif // OSUX_EDITOR_BEATMAPLABEL_H
