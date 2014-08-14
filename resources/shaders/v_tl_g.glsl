#version 150

/*
 * Copyright (C) 2014 SINTEF ICT,
 * Applied Mathematics, Norway.
 *
 * Contact information:
 * E-mail: eivind.fonn@sintef.no
 * SINTEF ICT, Department of Applied Mathematics,
 * P.O. Box 4760 Sluppen,
 * 7045 Trondheim, Norway.
 *
 * This file is part of BSGUI.
 *
 * BSGUI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * BSGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with GoTools. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In accordance with Section 7(b) of the GNU Affero General Public
 * License, a covered work must retain the producer line in every data
 * file that is created or manipulated using GoTools.
 *
 * Other Usage
 * You can be released from the requirements of the license by purchasing
 * a commercial license. Buying such a license is mandatory as soon as you
 * develop commercial activities involving the GoTools library without
 * disclosing the source code of your own applications.
 *
 * This file may be used in accordance with the terms contained in a
 * written agreement between you and SINTEF ICT.
 */

layout(lines) in;
layout(triangle_strip, max_vertices = 24) out;

in vec3 vColor[];

uniform float thickness;
uniform mat4 mvp;

out vec3 fColor;

void emitQuad(vec4 a, vec4 b, vec4 c, vec4 d)
{
    gl_Position = a;
    EmitVertex();

    gl_Position = b;
    EmitVertex();

    gl_Position = c;
    EmitVertex();

    gl_Position = d;
    EmitVertex();

    EndPrimitive();
}

void main(void)
{
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;

    vec3 p = p1 - p0;
    vec3 r = vec3(1.0, 1.0, 1.0);

    vec3 a = thickness * normalize(cross(p, r));
    vec3 b = thickness * normalize(cross(p, a));

    vec4 v0 = mvp * vec4(p0 - a - b, 1.0);
    vec4 v1 = mvp * vec4(p0 - a + b, 1.0);
    vec4 v2 = mvp * vec4(p0 + a - b, 1.0);
    vec4 v3 = mvp * vec4(p0 + a + b, 1.0);
    vec4 v4 = mvp * vec4(p1 - a - b, 1.0);
    vec4 v5 = mvp * vec4(p1 - a + b, 1.0);
    vec4 v6 = mvp * vec4(p1 + a - b, 1.0);
    vec4 v7 = mvp * vec4(p1 + a + b, 1.0);

    fColor = vColor[0];
    emitQuad(v0, v1, v2, v3);
    emitQuad(v4, v5, v6, v7);
    emitQuad(v0, v1, v4, v5);
    emitQuad(v2, v3, v6, v7);
    emitQuad(v1, v5, v3, v7);
    emitQuad(v0, v4, v2, v6);
}
