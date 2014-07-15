#include <QElapsedTimer>
#include <QTimer>
#include <QVector4D>

#include <QGLWidget>
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

private:
    DispObject obj;
    
    QOpenGLShaderProgram vcProgram;
    QOpenGLShaderProgram ccProgram;
    QOpenGLShaderProgram lnProgram;

    QTimer timer;
    QElapsedTimer elapsedTimer;
};

#endif /* GLWIDGET_H */
