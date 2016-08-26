#ifndef EDOSU_GL_H
#define EDOSU_GL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EDOSU_TYPE_GL (edosu_gl_get_type())

G_DECLARE_FINAL_TYPE(EdosuGL, edosu_gl, EDOSU, GL, GtkGLArea);

struct _EdosuGL
{
    GtkGLArea parent;
};

EdosuGL *edosu_gl_new(void);

G_END_DECLS

#endif //EDOSU_GL_H
