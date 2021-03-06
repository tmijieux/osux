#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <gst/gst.h>
#include <glib/gi18n.h>
#include <locale.h>

#include "vosu-app.h"

void init_locale_settings(void)
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "UTF-8");
    textdomain(PACKAGE);
}

int main(int argc, char *argv[])
{
    init_locale_settings();
    gtk_init(&argc, &argv);
    gst_init(&argc, &argv);
    return g_application_run(
        G_APPLICATION(vosu_application_new()), argc, argv);
}
