#include <cmath>

#include "dispobject.h"
#include "shaders.h"

#include "glwidget.h"


GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent),
                                      vcProgram(), ccProgram(), lnProgram(),
                                      obj()
{
    setFocusPolicy(Qt::ClickFocus);
}


void GLWidget::paintGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = (float) width() / height();

    QMatrix4x4 projection;
    projection.perspective(fov, aspect, 0.1, 100.0);

    QMatrix4x4 modelview;
    QVector3D eye = QVector3D(0, camPos, 0);
    modelview.lookAt(eye, eye + QVector3D(0, 1, 0), QVector3D(0, 0, 1));
    modelview.translate(QVector3D(0, 4, 0));
    modelview.rotate(inclination, QVector3D(1, 0, 0));
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


void GLWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() & Qt::Key_Control)
        ctrlPressed = true;
    if (event->key() & Qt::Key_Shift)
        shiftPressed = true;
}


void GLWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() & Qt::Key_Control)
        ctrlPressed = false;
    if (event->key() & Qt::Key_Shift)
        shiftPressed = false;
}


void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() & Qt::RightButton)
    {
        mouseTracking = true;
        mouseOrig = event->pos();
        azimuthOrig = azimuth;
        inclinationOrig = inclination;
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
        inclination = inclinationOrig + 180.0 * (event->pos().y() - mouseOrig.y()) / height();
        update();
    }
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    if (ctrlPressed)
    {
        fov /= exp(event->angleDelta().y() / 120.0 / 15.0);
        if (fov > 135.0)
            fov = 135.0;
    }
    else
        camPos += event->angleDelta().y() / 120.0 / 5.0;

    update();
}
