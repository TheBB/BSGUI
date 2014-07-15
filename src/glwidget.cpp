#include "dispobject.h"
#include "shaders.h"

#include "glwidget.h"


GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent),
                                      vcProgram(), ccProgram(), lnProgram(),
                                      obj()
{
}


void GLWidget::paintGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = (float) width() / height();

    QMatrix4x4 projection;
    projection.perspective(45.0, aspect, 0.1, 100.0);

    QMatrix4x4 modelview;
    modelview.lookAt(QVector3D(0, -4, 2), QVector3D(0, 0, 0), QVector3D(0, 0, 1));
    modelview.rotate(azimuth, QVector3D(0, 0, 1));

    obj.draw(projection, modelview, vcProgram, ccProgram, lnProgram);

    swapBuffers();
}


void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}


void GLWidget::initializeGL()
{
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!vcProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vsVaryingColor))
        close();
    if (!vcProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fsVaryingColor))
        close();
    if (!vcProgram.link())
        close();

    if (!ccProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vsConstantColor))
        close();
    if (!ccProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fsConstantColor))
        close();
    if (!ccProgram.link())
        close();

    if (!lnProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vsLines))
        close();
    if (!lnProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fsLines))
        close();
    if (!lnProgram.link())
        close();

    obj.init();
}


void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() & Qt::RightButton)
    {
        mouseTracking = true;
        mouseOrig = event->pos();
        azimuthOrig = azimuth;
    }
}


void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() & Qt::RightButton)
        mouseTracking = false;
}


void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseTracking)
    {
        azimuth = azimuthOrig + 360.0 * (event->pos().x() - mouseOrig.x()) / width();
        update();
    }
}
