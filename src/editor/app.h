#ifndef OSUX_EDITOR_APP_H
#define OSUX_EDITOR_APP_H

#include <gtk/gtk.h>
#include <locale.h>
#include <libintl.h>
#include "beatmap.h"

#define _(x) dgettext("osux", (x))


G_BEGIN_DECLS

#define OSUX_TYPE_EDITOR_APP (osux_editor_app_get_type ())

G_DECLARE_FINAL_TYPE (OsuxEditorApp, osux_editor_app,
                      OSUX, EDITOR_APP, GtkApplication)

OsuxEditorApp *osux_editor_app_new(void);
OsuxEditorBeatmap *
osux_editor_app_get_beatmap_by_page(OsuxEditorApp *app, GtkWidget *page);


G_END_DECLS

#endif //OSUX_EDITOR_APP_H
