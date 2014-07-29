#version 130

in vec3 vertexPosition;
in vec3 vertexColor;
uniform mat4 mvp;

out vec3 outColor;

void main(void)
{
    outColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
