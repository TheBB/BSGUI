#include "dispobject.h"
#include "shaders.h"

#include "glwidget.h"


GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent),
                                      vcProgram(), ccProgram(),
                                      obj(),
                                      timer(this)
{
}


void GLWidget::paintGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float el = elapsedTimer.elapsed() / 1000.0;
    float aspect = (float) width() / height();

    QMatrix4x4 projection;
    projection.perspective(45.0, aspect, 0.1, 100.0);

    QMatrix4x4 modelview;
    modelview.lookAt(QVector3D(0, -4, 2), QVector3D(0, 0, 0), QVector3D(0, 0, 1));
    modelview.rotate(360.0 * el / 10, QVector3D(0, 0, 1));

    obj.draw(elapsedTimer.elapsed(), projection, modelview, vcProgram, ccProgram);

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

    if (!ccProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vsLines))
        close();
    if (!ccProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fsLines))
        close();
    if (!ccProgram.link())
        close();

    obj.init();

    elapsedTimer.start();

    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer.start(25);
}
