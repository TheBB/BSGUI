#include "dispobject.h"


DispObject::DispObject() : vertexBuffer(QOpenGLBuffer::VertexBuffer),
                           colorBuffer(QOpenGLBuffer::VertexBuffer),
                           indexBuffer(QOpenGLBuffer::IndexBuffer)
{
}


void DispObject::init()
{
    float vertexData[] = {
        -1.0, -1.0,  1.0,
        1.0, -1.0,  1.0,
        1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,
        -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0,  1.0, -1.0,
        -1.0,  1.0, -1.0,
    };

    vertexBuffer.create();
    vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexBuffer.bind();
    vertexBuffer.allocate(vertexData, 8 * 3 * sizeof(float));
    
    float colorData[] = {
        0.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
    };

    colorBuffer.create();
    colorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    colorBuffer.bind();
    colorBuffer.allocate(colorData, 8 * 3 * sizeof(float));

    GLushort indexData[] = {
        0, 1, 2,
        2, 3, 0,
        3, 2, 6,
        6, 7, 3,
        7, 6, 5,
        5, 4, 7,
        4, 5, 1,
        1, 0, 4,
        4, 0, 3,
        3, 7, 4,
        1, 5, 6,
        6, 2, 1
    };

    indexBuffer.create();
    indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    indexBuffer.bind();
    indexBuffer.allocate(indexData, 12 * 3 * sizeof(GLushort));
}


void DispObject::draw(qint64 el, QOpenGLShaderProgram &prog)
{
    vertexBuffer.bind();
    prog.enableAttributeArray("vertexPosition");
    prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    colorBuffer.bind();
    prog.enableAttributeArray("vertexColor");
    prog.setAttributeBuffer("vertexColor", GL_FLOAT, 0, 3);

    indexBuffer.bind();

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
}
