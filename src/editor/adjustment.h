#ifndef OSUX_EDITOR_ADJUSTMENT_H
#define OSUX_EDITOR_ADJUSTMENT_H

#include <gtk/gtk.h>
#include <stdint.h>

G_BEGIN_DECLS

#define OSUX_TYPE_ADJUSTMENT (osux_adjustment_get_type())

G_DECLARE_FINAL_TYPE(OsuxAdjustment, osux_adjustment,
                     OSUX, ADJUSTMENT, GtkAdjustment);

struct _OsuxAdjustment
{
    GtkAdjustment parent;
    double *target_value;
    int64_t *target_value_i;
};

void osux_adjustment_set_target(OsuxAdjustment *adj, double *double_value_ptr);
void osux_adjustment_set_target_i(OsuxAdjustment *adj, int64_t *int_value_ptr);

GtkWidget *osux_adjustment_new(void);
GtkWidget *osux_adjustment_new_with_target(double *target_value);

G_END_DECLS

#endif // OSUX_EDITOR_ADJUSTMENT_H
