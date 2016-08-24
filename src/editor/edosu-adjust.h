#ifndef EDOSU_ADJUST_H
#define EDOSU_ADJUST_H

#include <gtk/gtk.h>
#include <stdint.h>

G_BEGIN_DECLS

#define EDOSU_TYPE_ADJUST (edosu_adjust_get_type())

G_DECLARE_FINAL_TYPE(EdosuAdjust, edosu_adjust,
                     EDOSU, ADJUST, GtkAdjustment);

struct _EdosuAdjust
{
    GtkAdjustment parent;
    double *target_value;
    int64_t *target_value_i;
};

void edosu_adjust_set_target(EdosuAdjust *adj, double *double_value_ptr);
void edosu_adjust_set_target_i(EdosuAdjust *adj, int64_t *int_value_ptr);

GtkWidget *edosu_ajust_new(void);
GtkWidget *edosu_adjust_new_with_target(double *target_value);

G_END_DECLS

#endif // EDOSU_ADJUST_H
