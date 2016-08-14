#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <locale.h>
#include <libintl.h>

#define _(x) dgettext("osux", (x))

#include "app.h"

void init_locale_settings(void)
{
    bindtextdomain("osux", "./locale/");
    textdomain("osux");
}

int main(int argc, char *argv[])
{
    init_locale_settings();
    return g_application_run(
        G_APPLICATION(osux_editor_app_new()), argc, argv);
}
