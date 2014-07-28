#include <QString>

#ifndef SHADERS_H
#define SHADERS_H

QString vsVaryingColor =
    "#version 130\n"
    "\n"
    "in vec3 vertexPosition;\n"
    "in vec3 vertexColor;\n"
    "uniform mat4 mvp;\n"
    "\n"
    "out vec3 outColor;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    outColor = vertexColor;\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

QString fsVaryingColor =
    "#version 130\n"
    "\n"
    "in vec3 outColor;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor = vec4(outColor, 1.0);\n"
    "}\n";

QString vsConstantColor =
    "#version 130\n"
    "\n"
    "in vec3 vertexPosition;\n"
    "in vec3 vertexNormal;\n"
    "uniform mat4 mvp;\n"
    "uniform float p;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = mvp * vec4(vertexPosition + p * vertexNormal, 1.0);\n"
    "}\n";

QString fsConstantColor =
    "#version 130\n"
    "\n"
    "in vec3 fsLines;\n"
    "uniform vec3 col;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor = vec4(col, 1.0);\n"
    "}\n";

#endif /* SHADERS_H */
