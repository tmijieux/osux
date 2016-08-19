#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <locale.h>

#include "app.h"
#include "beatmap.h"

void init_locale_settings(void)
{
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "utf-8");
    textdomain(PACKAGE);
}

int main(int argc, char *argv[])
{
    init_locale_settings();
    return g_application_run(
        G_APPLICATION(osux_editor_app_new()), argc, argv);
}
