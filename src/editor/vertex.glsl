#version 130

in vec2 position;
uniform vec3 color;
uniform mat4 model;
uniform mat4 proj;

out vec4 vertexColor;

void main()
{
    gl_Position = proj * model * vec4(position, 0.0, 1.0);
    vertexColor = vec4(color, 1.0);
}
