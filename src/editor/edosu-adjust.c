#include "edosu-adjust.h"

G_DEFINE_TYPE(EdosuAdjust, edosu_adjust, GTK_TYPE_ADJUSTMENT);

static gboolean
edosu_adjust_value_changed(GtkAdjustment *adj, gpointer user_data)
{
    (void) user_data;
    EdosuAdjust *osux_adj = EDOSU_ADJUST(adj);
    if (osux_adj->target_value != NULL)
        *osux_adj->target_value = gtk_adjustment_get_value(adj);
    if (osux_adj->target_value_i != NULL)
        *osux_adj->target_value_i = gtk_adjustment_get_value(adj);
    return FALSE;
}

static void
edosu_adjust_init(EdosuAdjust *adj)
{
    adj->target_value = NULL;
    g_signal_connect(GTK_ADJUSTMENT(adj), "value-changed",
                     G_CALLBACK( edosu_adjust_value_changed ), NULL);
}

static void
edosu_adjust_class_init(EdosuAdjustClass *klass)
{
    (void) klass;
}

void
edosu_adjust_set_target(EdosuAdjust *adj, double *double_value_ptr)
{
    g_assert(double_value_ptr != NULL);
    adj->target_value = double_value_ptr;
    gtk_adjustment_set_value(GTK_ADJUSTMENT(adj), *double_value_ptr);
}

void
edosu_adjust_set_target_i(EdosuAdjust *adj, int64_t *int_value_ptr)
{
    g_assert(int_value_ptr != NULL);
    adj->target_value_i = int_value_ptr;
    gtk_adjustment_set_value(GTK_ADJUSTMENT(adj), *int_value_ptr);
}

GtkWidget *edosu_adjust_new(void)
{
    return GTK_WIDGET( g_object_new(EDOSU_TYPE_ADJUST, NULL));
}

GtkWidget *edosu_adjust_new_with_target(double *target_value)
{
    EdosuAdjust *adj;
    adj = EDOSU_ADJUST( g_object_new(EDOSU_TYPE_ADJUST, NULL));
    edosu_adjust_set_target(adj, target_value);
    return GTK_WIDGET( adj);
}
