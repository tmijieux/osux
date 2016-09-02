#include <gtk/gtk.h>
#include <stdint.h>
#include <epoxy/gl.h>
#include <math.h>
#include <string.h>

#include "osux/compiler.h"
#include "osux/hitobject.h"
#include "osux/hit.h"

#include "edosu-gl.h"
#include "edosu-circle.h"
#include "glarea-error.h"

G_DEFINE_TYPE(EdosuGL, edosu_gl, GTK_TYPE_GL_AREA);

static void
matrix_ortho(float M[],
             float l/*left*/, float r/*right*/,
             float b/*bottom*/, float t/*top*/,
             float n/*near*/, float f/*far*/)
{
    M[0] = 2./(r-l); M[4] = 0.;       M[8]  = 0.;       M[12] = -(r+l)/(r-l);
    M[1] = 0.;       M[5] = 2./(t-b); M[9]  = 0.;       M[13] = -(t+b)/(t-b);
    M[2] = 0.;       M[6] = 0.;       M[10] = -2/(f-n); M[14] = -(f+n)/(f-n);
    M[3] = 0.;       M[7] = 0.;       M[11] = 0.;       M[15] = 1.;
}

static void matrix_multiply(float *m1, float *m2, float *res)
{
    memset(res, 0, sizeof*res * 16);
    for (unsigned i = 0; i < 4; ++i)
        for (unsigned k = 0; k < 4; ++k)
            for (unsigned j = 0; j < 4; ++j)
                res[j*4+i] += m1[k*4+i] * m2[j*4+k];
}

static void
matrix_identity(float M[16])
{
    /* initialize a matrix as an identity matrix */
    M[0] = 1.f; M[4] = 0.f;  M[8]  = 0.f; M[12] = 0.f;
    M[1] = 0.f; M[5] = 1.f;  M[9]  = 0.f; M[13] = 0.f;
    M[2] = 0.f; M[6] = 0.f;  M[10] = 1.f; M[14] = 0.f;
    M[3] = 0.f; M[7] = 0.f;  M[11] = 0.f; M[15] = 1.f;
}

static void matrix_translation(float *M, int x, int y)
{
    matrix_identity(M);
    M[12] = x;
    M[13] = y;
}

static void
matrix_scale(float M[16], float s)
{
    /* initialize a matrix as an identity matrix */
    M[0] =   s; M[4] = 0.f;  M[8]  = 0.f; M[12] = 0.f;
    M[1] = 0.f; M[5] =   s;  M[9]  = 0.f; M[13] = 0.f;
    M[2] = 0.f; M[6] = 0.f;  M[10] =   s; M[14] = 0.f;
    M[3] = 0.f; M[7] = 0.f;  M[11] = 0.f; M[15] = 1.f;
}

static void
matrix_rotation_angle(float M[], float phi, float theta, float psi)
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
    // (opengl have column major layout)
    M[0] = c3c2;  M[4] = s3c1 + c3s2s1; M[8] = s3s1 - c3s2c1; M[12] = 0.f;
    M[1] = -s3c2; M[5] = c3c1 - s3s2s1; M[9] = c3s1 + s3s2c1; M[13] = 0.f;
    M[2] = s2;    M[6] = -c2s1;         M[10] = c2c1;         M[14] = 0.f;
    M[3] = 0.f;   M[7] = 0.f;           M[11] = 0.f;          M[15] = 1.f;
}

static void
init_buffers(guint position_index, guint *vao_out)
{
    guint vao, buffer, index_buffer;

    /* we need to create a VAO to store the other buffers */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /* this is the VBO that holds the vertex data */
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 edosu_circle_data_size, edosu_circle_data, GL_STATIC_DRAW);

    /* enable and set the position attribute */
    glEnableVertexAttribArray(position_index);
    glVertexAttribPointer(position_index, 2, GL_FLOAT, GL_FALSE,
                          sizeof(struct vertex_data),
                          (GLvoid*)(G_STRUCT_OFFSET(struct vertex_data, position)));
    /* this is the VBO that holds the vertex index */
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 edosu_circle_index_size, edosu_circle_index, GL_STATIC_DRAW);

    /* reset the state; we will re-enable the VAO when needed */
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /* the VBO is referenced by the VAO */
    glDeleteBuffers(1, &buffer);

    if (vao_out != NULL)
        *vao_out = vao;
}

static guint
create_shader(int shader_type,
              char const *source,
              GError **error,
              guint *shader_out)
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
             guint   *proj_location_out,
             guint   *model_location_out,
             guint   *position_location_out,
             guint   *color_location_out,
             GError **error)
{
    GBytes *source;
    guint program = 0;
    guint proj_location = 0;
    guint model_location = 0;
    guint vertex = 0, fragment = 0;
    guint position_location = 0;
    guint color_location = 0;

    /* load the vertex shader */
    source = g_resources_lookup_data("/org/osux/edosu/vertex.glsl", 0, NULL);
    create_shader(GL_VERTEX_SHADER,
                  g_bytes_get_data(source, NULL), error, &vertex);
    g_bytes_unref(source);
    if (vertex == 0)
        goto out;

    /* load the fragment shader */
    source = g_resources_lookup_data("/org/osux/edosu/fragment.glsl", 0, NULL);
    create_shader(GL_FRAGMENT_SHADER,
                  g_bytes_get_data(source, NULL), error, &fragment);
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

    /* get the location of the uniform */
    proj_location = glGetUniformLocation(program, "proj");
    model_location = glGetUniformLocation(program, "model");
    color_location = glGetUniformLocation(program, "color");

    /* get the location of the "position" attribute */
    position_location = glGetAttribLocation(program, "position");

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
    if (proj_location_out != NULL)
        *proj_location_out = proj_location;
    if (model_location_out != NULL)
        *model_location_out = model_location;
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
                       &self->proj_location,
                       &self->model_location,
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
    glUseProgram(self->program);
    /* set the color */
    float color[] = { 1.0, 0.0, 1.0 };
    glUniform3fv(self->color_index, 1, color);
    glUseProgram(0);

    /* initialize the vertex buffers */
    init_buffers(self->position_index, &self->vao);
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
        glDeleteProgram(self->program);
    return FALSE;
}

static gint
sort_object_offset(gconstpointer _a, gconstpointer _b, gpointer UNUSED user_data)
{
    return ((osux_hitobject*)_a)->offset - ((osux_hitobject*)_b)->offset;
}

static bool
object_is_approach_time(osux_hitobject *ho, int64_t position)
{
    int approach_time;
    int64_t local_offset;

    approach_time = osux_get_approach_time(9.0, 0);
    local_offset = ho->offset - position;
    return (local_offset > 0 && local_offset < approach_time);
}

static void draw_object(EdosuGL *self, osux_hitobject *ho)
{
    float S[16];
    float M[16];

    matrix_scale(S, 40.0);
    matrix_translation(M, ho->x-256, ho->y-256);
    matrix_multiply(M, S, self->model);

    glUniformMatrix4fv(self->model_location, 1, GL_FALSE, &(self->model[0]));
    glDrawElements(GL_TRIANGLES,
                   edosu_circle_index_size, GL_UNSIGNED_BYTE, (GLvoid*) 0);
}

static void draw_objects(EdosuGL *self)
{
    GSequenceIter *iter;
    osux_hitobject key, *obj;

    if (self->program == 0 || self->vao == 0 || self->hitobjects == NULL)
        return;
    glUseProgram(self->program);
    glBindVertexArray(self->vao);

    if (self->proj_changed) {
        glUniformMatrix4fv(self->proj_location, 1, GL_FALSE, &(self->proj[0]));
        self->proj_changed = FALSE;
    }

    key.offset = self->position-1;
    iter = g_sequence_search(self->hitobjects, &key, sort_object_offset, NULL);
    iter = g_sequence_iter_next(iter);

    while (!g_sequence_iter_is_end(iter) &&
           (obj = g_sequence_get(iter)) &&
           object_is_approach_time(obj, self->position))
    {
        draw_object(self, obj);
        iter = g_sequence_iter_next(iter);
    }
    glBindVertexArray(0);
    glUseProgram(0);
}

static gboolean render(EdosuGL *self)
{
    glClearColor(.5, .5, .5, 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    draw_objects(self);
    return TRUE;
}

void edosu_gl_set_position(EdosuGL *self, int64_t pos)
{
    self->position = pos;
    gtk_widget_queue_draw(GTK_WIDGET(self));
}

void edosu_gl_set_hit_objects(EdosuGL *self, GSequence *hitobjects)
{
    self->hitobjects = hitobjects;
}

static void
edosu_gl_init(EdosuGL *self)
{
    float h = 600;
    float w = 1.7777777 * h;

    matrix_ortho(self->proj, -w/2, w/2, -h/2, h/2, -w/2, w/2);
    self->proj_changed = TRUE;

    matrix_scale(self->model, 50.0);
    self->model_changed = TRUE;

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
