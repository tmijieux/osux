#ifndef EDOSU_INSPECTOR_H
#define EDOSU_INSPECTOR_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EDOSU_TYPE_INSPECTOR (edosu_inspector_get_type())

G_DECLARE_FINAL_TYPE(EdosuInspector, edosu_inspector, EDOSU, INSPECTOR, GtkBox);

struct _EdosuInspector
{
    GtkBox parent;
};

EdosuInspector *edosu_inspector_new(void);

G_END_DECLS

#endif //EDOSU_INSPECTOR_H
