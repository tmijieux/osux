#ifndef EDOSU_VIEW_H
#define EDOSU_VIEW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EDOSU_TYPE_VIEW (edosu_view_get_type())

G_DECLARE_FINAL_TYPE(EdosuView, edosu_view, EDOSU, VIEW, GtkBox);

struct _EdosuView
{
    GtkBox parent;
};

EdosuView *edosu_view_new(void);

G_END_DECLS

#endif //EDOSU_VIEW_H
