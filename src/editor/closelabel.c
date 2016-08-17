#include <gtk/gtk.h>
#include <stdint.h>

#include "app.h"
#include "app_win.h"
#include "beatmap.h"
#include "popsearch.h"
#include "closelabel.h"

G_DEFINE_TYPE(OsuxCloseLabel, osux_close_label, GTK_TYPE_BOX);

static void
osux_close_label_init(OsuxCloseLabel *label)
{
    gtk_widget_init_template(GTK_WIDGET(label));
}

static void
osux_close_label_class_init(OsuxCloseLabelClass *klass)
{
    GtkWidgetClass *wklass = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        wklass, "/org/osux/editor/ui/OsuxCloseLabel.glade");
    gtk_widget_class_bind_template_child(wklass, OsuxCloseLabel, label_value);
}

GtkWidget *osux_close_label_new(gchar const *label_text)
{
    OsuxCloseLabel *label;
    label = OSUX_CLOSE_LABEL( g_object_new(OSUX_TYPE_CLOSE_LABEL, NULL));
    gtk_label_set_text(label->label_value, label_text);
    return GTK_WIDGET( label );
}

gboolean osux_close_button_clicked(GtkWidget *button, gpointer user_data)
{
    (void) button;
    (void) user_data;
    return FALSE;
}
