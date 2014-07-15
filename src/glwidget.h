#include <QElapsedTimer>
#include <QTimer>
#include <QVector4D>

#include <QGLWidget>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>

#include "dispobject.h"

#ifndef GLWIDGET_H
#define GLWIDGET_H

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = NULL);
    virtual ~GLWidget() { }

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    DispObject obj;

    QOpenGLShaderProgram vcProgram;
    QOpenGLShaderProgram ccProgram;
    QOpenGLShaderProgram lnProgram;

    double azimuthOrig = 45.0;
    double azimuth = 45.0;

    bool mouseTracking = false;
    QPoint mouseOrig;
};

#endif /* GLWIDGET_H */
