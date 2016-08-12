#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <locale.h>
#include <libintl.h>

#define _(x) dgettext("osux", (x))

void init_locale_settings(void)
{
    bindtextdomain("osux", "./locale/");
    textdomain("osux");
}

GtkWidget *popover;

gboolean on_main_window_key_press(
    GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    switch (event->keyval) {
    case GDK_KEY_space:
        if (gtk_widget_is_visible((GtkWidget*) popover))
            gtk_widget_hide((GtkWidget*) popover);
        else
            gtk_widget_show_all((GtkWidget*) popover);
        printf(_("hello\n"));
        return TRUE;
        break;
    default:
        break;
    };
    return FALSE;
}

int main(int argc, char *argv[])
{
    GtkBuilder *builder;

    init_locale_settings();
    gtk_init(&argc, &argv);
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "fen1.ui", NULL);
    gtk_builder_connect_signals(builder, NULL);
    popover = GTK_WIDGET( gtk_builder_get_object(builder, "popover1") );
    gtk_main();

    return 0;
}
