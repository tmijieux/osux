#include <gtk/gtk.h>
#include <stdint.h>

#include "edosu-palette.h"

G_DEFINE_TYPE(EdosuPalette, edosu_palette, GTK_TYPE_SCROLLED_WINDOW);

static void
edosu_palette_init(EdosuPalette *label)
{
    gtk_widget_init_template(GTK_WIDGET(label));
}

static void
edosu_palette_class_init(EdosuPaletteClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    gtk_widget_class_set_template_from_resource(
        widget_class, "/org/osux/edosu/ui/EdosuPalette.glade");
}

EdosuPalette*edosu_palette_new(void)
{
    return EDOSU_PALETTE(g_object_new(EDOSU_TYPE_PALETTE, NULL));
}
