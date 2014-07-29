#version 130

in vec3 vertexPosition;
in vec3 vertexNormal;
uniform mat4 mvp;
uniform float p;

void main(void)
{
    gl_Position = mvp * vec4(vertexPosition + p * vertexNormal, 1.0);
}
