#version 130

in vec3 fsLines;
uniform vec3 col;

void main(void)
{
    gl_FragColor = vec4(col, 1.0);
}
