#ifndef EDOSU_PROPERTIES_H
#define EDOSU_PROPERTIES_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EDOSU_TYPE_PROPERTIES (edosu_properties_get_type())
G_DECLARE_FINAL_TYPE(EdosuProperties, edosu_properties, EDOSU, PROPERTIES, GtkNotebook);

struct _EdosuProperties
{
    GtkNotebook parent;
};

EdosuProperties *edosu_properties_new(void);

G_END_DECLS

#endif //EDOSU_PROPERTIES_H
