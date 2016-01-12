#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <locale.h>

#include <libintl.h>
#define _(x) dgettext("messages", (x))

static void print_hello(GtkWidget *widget, gpointer data)
{
    g_print("Hello World\n");
}

int main(int argc, char *argv[])
{
    putenv("LANG=fr_FR.utf-8");
    setlocale(LC_ALL, "fr_FR.utf-8");
    bindtextdomain("messages", "./locale/");
    textdomain("messages");

    puts(_("Hello World"));

    gtk_init(&argc, &argv);

    GtkBuilder *builder;
    GObject *window;
    
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "ui/interface.glade", NULL);
    window = gtk_builder_get_object(builder, "window1");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    gtk_main();

    return 0;
}
