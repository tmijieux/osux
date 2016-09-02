#ifndef EDOSU_GL_H
#define EDOSU_GL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EDOSU_TYPE_GL (edosu_gl_get_type())

G_DECLARE_FINAL_TYPE(EdosuGL, edosu_gl, EDOSU, GL, GtkGLArea);

struct _EdosuGL
{
    GtkGLArea parent;

    float model[16];
    float proj[16];
    gboolean model_changed;
    gboolean proj_changed;

    guint vao;
    guint program;

    guint proj_location;
    guint model_location;
    guint position_index;
    guint color_index;

    GSequence *hitobjects;
    int64_t position;
};

EdosuGL *edosu_gl_new(void);
void edosu_gl_set_position(EdosuGL *self, int64_t pos);
void edosu_gl_set_hit_objects(EdosuGL *self, GSequence *hitobjects);

G_END_DECLS

#endif //EDOSU_GL_H
