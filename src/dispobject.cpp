#include <QVector4D>

#include "dispobject.h"


DispObject::DispObject() : vertexBuffer(QOpenGLBuffer::VertexBuffer),
                           colorBuffer(QOpenGLBuffer::VertexBuffer),
                           faceIndexBuffer(QOpenGLBuffer::IndexBuffer),
                           lineIndexBuffer(QOpenGLBuffer::IndexBuffer)
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

    GLushort faceIndexData[] = {
        0, 1, 2, 3,
        4, 5, 6, 7,
        0, 4, 7, 3,
        1, 2, 6, 5,
        0, 1, 5, 4,
        3, 2, 6, 7,
    };

    faceIndexBuffer.create();
    faceIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    faceIndexBuffer.bind();
    faceIndexBuffer.allocate(faceIndexData, 6 * 4 * sizeof(GLushort));

    GLushort lineIndexData[] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        1, 5,
        2, 6,
        3, 7,
    };

    lineIndexBuffer.create();
    lineIndexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    lineIndexBuffer.bind();
    lineIndexBuffer.allocate(lineIndexData, 12 * 2 * sizeof(GLushort));
}


void DispObject::draw(qint64 el, QMatrix4x4 &proj, QMatrix4x4 &mv,
                      QOpenGLShaderProgram &vprog, QOpenGLShaderProgram &cprog)
{
    vprog.bind();

    vertexBuffer.bind();
    vprog.enableAttributeArray("vertexPosition");
    vprog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    colorBuffer.bind();
    vprog.enableAttributeArray("vertexColor");
    vprog.setAttributeBuffer("vertexColor", GL_FLOAT, 0, 3);

    faceIndexBuffer.bind();

    QMatrix4x4 mvp = proj * mv;
    vprog.setUniformValue("mvp", mvp);
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_SHORT, 0);


    cprog.bind();

    vertexBuffer.bind();
    cprog.enableAttributeArray("vertexPosition");
    cprog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    lineIndexBuffer.bind();

    cprog.setUniformValue("proj", proj);
    cprog.setUniformValue("mv", mv);
    cprog.setUniformValue("col", QVector4D(0.0, 0.0, 0.0, 1.0));
    glLineWidth(2.0);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, 0);
}
