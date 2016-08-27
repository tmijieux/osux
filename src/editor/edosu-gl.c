#include <gtk/gtk.h>
#include <stdint.h>
#include <epoxy/gl.h>
#include <math.h>

#include "edosu-gl.h"
#include "glarea-error.h"

G_DEFINE_TYPE(EdosuGL, edosu_gl, GTK_TYPE_GL_AREA);

/* position and color information for each vertex */
struct vertex_info {
    float position[3];
    float color[3];
};

/* the vertex data is constant */
static const struct vertex_info vertex_data[] = {
    { {  0.0f,  0.500f, 0.0f }, { 1.f, 0.f, 0.f } },
    { {  0.5f, -0.366f, 0.0f }, { 0.f, 1.f, 0.f } },
    { { -0.5f, -0.366f, 0.0f }, { 0.f, 0.f, 1.f } },
};

static void
init_buffers(guint  position_index,
             guint  color_index,
             guint *vao_out)
{
    guint vao, buffer;

    /* we need to create a VAO to store the other buffers */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /* this is the VBO that holds the vertex data */
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

    /* enable and set the position attribute */
    glEnableVertexAttribArray(position_index);
    glVertexAttribPointer(position_index, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct vertex_info),
                          (GLvoid *)(G_STRUCT_OFFSET(struct vertex_info, position)));

    /* enable and set the color attribute */
    glEnableVertexAttribArray(color_index);
    glVertexAttribPointer(color_index, 3, GL_FLOAT, GL_FALSE,
                          sizeof(struct vertex_info),
                          (GLvoid *)(G_STRUCT_OFFSET(struct vertex_info, color)));

    /* reset the state; we will re-enable the VAO when needed */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /* the VBO is referenced by the VAO */
    glDeleteBuffers(1, &buffer);

    if (vao_out != NULL)
        *vao_out = vao;
}

static guint
create_shader(int          shader_type,
              const char  *source,
              GError     **error,
              guint       *shader_out)
{
    guint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        int log_len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);

        char *buffer = g_malloc(log_len + 1);
        glGetShaderInfoLog(shader, log_len, NULL, buffer);

        g_set_error(error, GLAREA_ERROR, GLAREA_ERROR_SHADER_COMPILATION,
                    "Compilation failure in %s shader: %s",
                    shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
                    buffer);

        g_free(buffer);
        glDeleteShader(shader);
        shader = 0;
    }

    if (shader_out != NULL)
        *shader_out = shader;

    return shader != 0;
}

static gboolean
init_shaders(guint   *program_out,
             guint   *mvp_location_out,
             guint   *position_location_out,
             guint   *color_location_out,
             GError **error)
{
    GBytes *source;
    guint program = 0;
    guint mvp_location = 0;
    guint vertex = 0, fragment = 0;
    guint position_location = 0;
    guint color_location = 0;

    /* load the vertex shader */
    source = g_resources_lookup_data(
        "/org/osux/edosu/glarea-vertex.glsl", 0, NULL);
    create_shader(GL_VERTEX_SHADER, g_bytes_get_data(source, NULL), error, &vertex);
    g_bytes_unref(source);
    if (vertex == 0)
        goto out;

    /* load the fragment shader */
    source = g_resources_lookup_data(
        "/org/osux/edosu/glarea-fragment.glsl", 0, NULL);
    create_shader(GL_FRAGMENT_SHADER, g_bytes_get_data(source, NULL),
                  error, &fragment);
    g_bytes_unref(source);
    if (fragment == 0)
        goto out;

    /* link the vertex and fragment shaders together */
    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    int status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        int log_len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);

        char *buffer = g_malloc(log_len + 1);
        glGetProgramInfoLog(program, log_len, NULL, buffer);

        g_set_error(error, GLAREA_ERROR, GLAREA_ERROR_SHADER_LINK,
                    "Linking failure in program: %s", buffer);

        g_free(buffer);

        glDeleteProgram(program);
        program = 0;

        goto out;
    }

    /* get the location of the "mvp" uniform */
    mvp_location = glGetUniformLocation(program, "mvp");

    /* get the location of the "position" and "color" attributes */
    position_location = glGetAttribLocation(program, "position");
    color_location = glGetAttribLocation(program, "color");

    /* the individual shaders can be detached and destroyed */
    glDetachShader(program, vertex);
    glDetachShader(program, fragment);

 out:
    if (vertex != 0)
        glDeleteShader(vertex);
    if (fragment != 0)
        glDeleteShader(fragment);

    if (program_out != NULL)
        *program_out = program;
    if (mvp_location_out != NULL)
        *mvp_location_out = mvp_location;
    if (position_location_out != NULL)
        *position_location_out = position_location;
    if (color_location_out != NULL)
        *color_location_out = color_location;

    return program != 0;
}


static gboolean
realize(EdosuGL *self)
{
    GtkGLArea *gl_area = GTK_GL_AREA(self);
    /* we need to ensure that the GdkGLContext is set before calling GL API */
    gtk_gl_area_make_current(gl_area);

    /* if the GtkGLArea is in an error state we don't do anything */
    if (gtk_gl_area_get_error(gl_area) != NULL)
        return TRUE;

    /* initialize the shaders and retrieve the program data */
    GError *error = NULL;
    if (!init_shaders (&self->program,
                       &self->mvp_location,
                       &self->position_index,
                       &self->color_index,
                       &error))
    {
        /* set the GtkGLArea in error state, so we'll see the error message
         * rendered inside the viewport
         */
        gtk_gl_area_set_error(gl_area, error);
        g_error_free(error);
        return FALSE;
    }

    /* initialize the vertex buffers */
    init_buffers(self->position_index, self->color_index, &self->vao);
    return FALSE;
}

static gboolean
unrealize(EdosuGL *self)
{
    GtkGLArea *gl_area = GTK_GL_AREA(self);
    
    /* we need to ensure that the GdkGLContext is set before calling GL API */
    gtk_gl_area_make_current(gl_area);

    /* skip everything if we're in error state */
    if (gtk_gl_area_get_error(gl_area) != NULL)
        return TRUE;

    /* destroy all the resources we created */
    if (self->vao != 0)
        glDeleteVertexArrays(1, &self->vao);
    if (self->program != 0)
        glDeleteProgram (self->program);
    return FALSE;
}

static void
draw_triangle(EdosuGL *self)
{
    if (self->program == 0 || self->vao == 0)
        return;

    /* load our program */
    glUseProgram(self->program);

    /* update the "mvp" matrix we use in the shader */
    glUniformMatrix4fv(self->mvp_location, 1, GL_FALSE, &(self->mvp[0]));

    /* use the buffers in the VAO */
    glBindVertexArray(self->vao);

    /* draw the three vertices as a triangle */
    glDrawArrays(GL_TRIANGLES, 0, 3);

    /* we finished using the buffers and program */
    glBindVertexArray(0);
    glUseProgram(0);
}

static gboolean
render(EdosuGL *self)
{
    glClearColor(.5, .5, .5, 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    draw_triangle(self);
    glFlush();
    
    return TRUE;
}

static void
init_mvp (float *res)
{
  /* initialize a matrix as an identity matrix */
  res[0] = 1.f; res[4] = 0.f;  res[8] = 0.f; res[12] = 0.f;
  res[1] = 0.f; res[5] = 1.f;  res[9] = 0.f; res[13] = 0.f;
  res[2] = 0.f; res[6] = 0.f; res[10] = 1.f; res[14] = 0.f;
  res[3] = 0.f; res[7] = 0.f; res[11] = 0.f; res[15] = 1.f;
}

static void
compute_mvp (float *res,
             float  phi,
             float  theta,
             float  psi)
{
  float x = phi * (G_PI / 180.f);
  float y = theta * (G_PI / 180.f);
  float z = psi * (G_PI / 180.f);
  float c1 = cosf (x), s1 = sinf (x);
  float c2 = cosf (y), s2 = sinf (y);
  float c3 = cosf (z), s3 = sinf (z);
  float c3c2 = c3 * c2;
  float s3c1 = s3 * c1;
  float c3s2s1 = c3 * s2 * s1;
  float s3s1 = s3 * s1;
  float c3s2c1 = c3 * s2 * c1;
  float s3c2 = s3 * c2;
  float c3c1 = c3 * c1;
  float s3s2s1 = s3 * s2 * s1;
  float c3s1 = c3 * s1;
  float s3s2c1 = s3 * s2 * c1;
  float c2s1 = c2 * s1;
  float c2c1 = c2 * c1;
  
  /* apply all three Euler angles rotations using the three matrices:
   *
   * ⎡  c3 s3 0 ⎤ ⎡ c2  0 -s2 ⎤ ⎡ 1   0  0 ⎤
   * ⎢ -s3 c3 0 ⎥ ⎢  0  1   0 ⎥ ⎢ 0  c1 s1 ⎥
   * ⎣   0  0 1 ⎦ ⎣ s2  0  c2 ⎦ ⎣ 0 -s1 c1 ⎦
   */
  res[0] = c3c2;  res[4] = s3c1 + c3s2s1;  res[8] = s3s1 - c3s2c1; res[12] = 0.f;
  res[1] = -s3c2; res[5] = c3c1 - s3s2s1;  res[9] = c3s1 + s3s2c1; res[13] = 0.f;
  res[2] = s2;    res[6] = -c2s1;         res[10] = c2c1;          res[14] = 0.f;
  res[3] = 0.f;   res[7] = 0.f;           res[11] = 0.f;           res[15] = 1.f;
}

static void
edosu_gl_init(EdosuGL *self)
{
    init_mvp(self->mvp);
    g_signal_connect(self, "render", G_CALLBACK(render), NULL);
    g_signal_connect(self, "realize", G_CALLBACK(realize), NULL);
    g_signal_connect(self, "unrealize", G_CALLBACK(unrealize), NULL);
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
