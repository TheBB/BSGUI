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

QString vsLines =
    "#version 130\n"
    "\n"
    "in vec3 vertexPosition;\n"
    "uniform mat4 proj;\n"
    "uniform mat4 mv;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    vec4 v = mv * vec4(vertexPosition, 1.0);\n"
    "    v.xyz = v.xyz * 0.999;\n"
    "    gl_Position = proj * v;\n"
    "}\n";

QString fsLines =
    "#version 130\n"
    "\n"
    "uniform vec4 col;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor = col;\n"
    "}\n";

#endif /* SHADERS_H */
