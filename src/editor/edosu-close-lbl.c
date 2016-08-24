#include <gtk/gtk.h>
#include <stdint.h>

#include "edosu-app.h"
#include "edosu-win.h"
#include "edosu-beatmap.h"
#include "edosu-close-lbl.h"


struct _EdosuCloseLabel
{
    GtkBox parent;
    GtkLabel *label;
    GtkWidget *close_button;

    EdosuApplication *app;
    EdosuBeatmap *beatmap;
};

G_DEFINE_TYPE(EdosuCloseLabel, edosu_close_label, GTK_TYPE_BOX);

static void
edosu_close_label_init(EdosuCloseLabel *label)
{
    gtk_widget_init_template(GTK_WIDGET(label));
}

static void
edosu_close_label_class_init(EdosuCloseLabelClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        widget_class, "/org/osux/edosu/ui/EdosuCloseLabel.glade");
    gtk_widget_class_bind_template_child(widget_class, EdosuCloseLabel, label);
    gtk_widget_class_bind_template_child(widget_class, EdosuCloseLabel, close_button);
}

gboolean on_close_button_clicked_cb(EdosuCloseLabel *cl)
{
    edosu_application_close_beatmap(cl->app, cl->beatmap);
    return FALSE;
}

void edosu_close_label_set_text(EdosuCloseLabel *close_label, gchar const *text)
{
    gtk_label_set_text(close_label->label, text);
}

GtkWidget *edosu_close_label_new(gchar const *name,
                                 EdosuApplication *app,
                                 EdosuBeatmap *beatmap)
{
    EdosuCloseLabel *cl;
    cl = EDOSU_CLOSE_LABEL( g_object_new(EDOSU_TYPE_CLOSE_LABEL, NULL));
    gtk_label_set_text(cl->label, name);
    cl->beatmap = beatmap;
    cl->app = app;
    g_signal_connect_swapped(cl->close_button, "clicked",
                             G_CALLBACK(on_close_button_clicked_cb), cl);
    return GTK_WIDGET(cl);
}
