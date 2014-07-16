#include <set>

#include <QVector3D>
#include <QVector4D>

#include <QGLWidget>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QSize>
#include <QWheelEvent>

#include "DispObject.h"

#ifndef GLWIDGET_H
#define GLWIDGET_H

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = NULL);
    virtual ~GLWidget();

    QSize sizeHint() const { return QSize(640, 480); }

    double inclination() { return _inclination; }
    void setInclination(double val, bool fromMouse);

    double azimuth() { return _azimuth; }
    void setAzimuth(double val, bool fromMouse);

signals:
    void inclinationChanged(double val, bool fromMouse);
    void azimuthChanged(double val, bool fromMouse);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    void matrices(QMatrix4x4 *, QMatrix4x4 *);

    QOpenGLShaderProgram vcProgram;
    QOpenGLShaderProgram ccProgram;
    QOpenGLShaderProgram lnProgram;

    std::set<DispObject *> objects;
    DispObject *selectedObject = NULL;

    bool shiftPressed = false;
    bool ctrlPressed = false;

    double camPos = -8.0;
    QVector3D worldTrans;

    double fov = 45.0;

    double inclinationOrig;
    double _inclination = 30.0;

    double azimuthOrig = 45.0;
    double _azimuth = 45.0;

    bool cameraTracking = false;
    QPoint cameraOrig;
};

#endif /* GLWIDGET_H */
