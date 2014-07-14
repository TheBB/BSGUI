#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#ifndef DISPOBJECT_H
#define DISPOBJECT_H

class DispObject
{
public:
    DispObject();
    virtual ~DispObject() { }

    void init();
    void draw(qint64, QMatrix4x4&, QMatrix4x4&, QOpenGLShaderProgram&, QOpenGLShaderProgram&);

private:
    QOpenGLBuffer vertexBuffer;
    QOpenGLBuffer colorBuffer;
    QOpenGLBuffer faceIndexBuffer;
    QOpenGLBuffer lineIndexBuffer;
};

#endif /* DISPOBJECT_H */
