#ifndef EDOSU_PALETTE_H
#define EDOSU_PALETTE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EDOSU_TYPE_PALETTE (edosu_palette_get_type())

G_DECLARE_FINAL_TYPE(EdosuPalette, edosu_palette, EDOSU, PALETTE, GtkScrolledWindow);

struct _EdosuPalette
{
    GtkScrolledWindow parent;
    GtkToolPalette *palette;
};

EdosuPalette *edosu_palette_new(void);

G_END_DECLS

#endif //EDOSU_PALETTE_H
