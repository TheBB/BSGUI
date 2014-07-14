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
    void draw(qint64 el, QOpenGLShaderProgram &prog);

private:
    QOpenGLBuffer vertexBuffer;
    QOpenGLBuffer colorBuffer;
    QOpenGLBuffer indexBuffer;
};

#endif /* DISPOBJECT_H */
