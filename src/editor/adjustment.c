#include "adjustment.h"

G_DEFINE_TYPE(OsuxAdjustment, osux_adjustment, GTK_TYPE_ADJUSTMENT);

static gboolean
osux_adjustment_value_changed(GtkAdjustment *adj, gpointer user_data)
{
    (void) user_data;
    OsuxAdjustment *osux_adj = OSUX_ADJUSTMENT(adj);
    if (osux_adj->target_value != NULL)
        *osux_adj->target_value = gtk_adjustment_get_value(adj);
    if (osux_adj->target_value_i != NULL)
        *osux_adj->target_value_i = gtk_adjustment_get_value(adj);
    return FALSE;
}

static void
osux_adjustment_init(OsuxAdjustment *adj)
{
    adj->target_value = NULL;
    g_signal_connect(GTK_ADJUSTMENT(adj), "value-changed",
                     G_CALLBACK( osux_adjustment_value_changed ), NULL);
}

static void
osux_adjustment_class_init(OsuxAdjustmentClass *klass)
{
    (void) klass;
}

void
osux_adjustment_set_target(OsuxAdjustment *adj, double *double_value_ptr)
{
    g_assert(double_value_ptr != NULL);
    adj->target_value = double_value_ptr;
    gtk_adjustment_set_value(GTK_ADJUSTMENT(adj), *double_value_ptr);
}

void
osux_adjustment_set_target_i(OsuxAdjustment *adj, int64_t *int_value_ptr)
{
    g_assert(int_value_ptr != NULL);
    adj->target_value_i = int_value_ptr;
    gtk_adjustment_set_value(GTK_ADJUSTMENT(adj), *int_value_ptr);
}

GtkWidget *osux_adjustment_new(void)
{
    return GTK_WIDGET( g_object_new(OSUX_TYPE_ADJUSTMENT, NULL));
}

GtkWidget *osux_adjustment_new_with_target(double *target_value)
{
    OsuxAdjustment *adj;
    adj = OSUX_ADJUSTMENT( g_object_new(OSUX_TYPE_ADJUSTMENT, NULL));
    osux_adjustment_set_target(adj, target_value);
    return GTK_WIDGET( adj);
}
