#include <gtk/gtk.h>
#include <stdint.h>

#include "edosu-gl.h"
#include "epoxy/gl.h"

G_DEFINE_TYPE(EdosuGL, edosu_gl, GTK_TYPE_GL_AREA);

static gboolean
realize(GtkGLArea *gl_area)
{
    GError *error;
    gtk_gl_area_make_current(gl_area);

    error = gtk_gl_area_get_error(gl_area);
    if (error != NULL) {
        printf("error: %s\n", error->message);
        return TRUE;
    }
    
    return FALSE;
}

static gboolean
render(GtkGLArea *gl_area, GdkGLContext *context)
{
    (void) context;
    (void) gl_area;
    glClearColor(.5, .5, .5, 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    return TRUE;
}

static void
edosu_gl_init(EdosuGL *self)
{
    g_signal_connect(self, "render", G_CALLBACK(render), NULL);
    g_signal_connect(self, "realize", G_CALLBACK(realize), NULL);
}

static void
edosu_gl_class_init(EdosuGLClass *klass)
{
    (void) klass;
}

EdosuGL *edosu_gl_new(void)
{
    return EDOSU_GL(g_object_new(EDOSU_TYPE_GL, NULL));
}
