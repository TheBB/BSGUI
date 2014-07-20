#include <mutex>
#include <set>

#include <QVector3D>
#include <QVector4D>

#include <QGLWidget>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QSize>
#include <QWheelEvent>

#include "ObjectSet.h"
#include "DispObject.h"

#ifndef _GLWIDGET_H_
#define _GLWIDGET_H_

#define MAX_FOV 135.0

enum direction { POSX, NEGX, POSY, NEGY, POSZ, NEGZ };
enum preset { VIEW_TOP, VIEW_BOTTOM, VIEW_LEFT, VIEW_RIGHT, VIEW_FRONT, VIEW_BACK, VIEW_FREE };

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(ObjectSet *oSet, QWidget *parent = NULL);
    virtual ~GLWidget();

    std::mutex m;

    QSize sizeHint() const;

    void centerOnSelected();
    void initializeObject(DispObject *obj);

    inline double inclination() { return _inclination; }
    void setInclination(double val, bool fromMouse);

    inline double azimuth() { return _azimuth; }
    void setAzimuth(double val, bool fromMouse);

    inline double roll() { return _roll; }
    void setRoll(double roll, bool fromMouse);

    inline double fov() { return _fov; }
    void setFov(double val, bool fromMouse);

    inline double zoom() { return _zoom; }
    void setZoom(double val, bool fromMouse);

    inline QVector3D lookAt() { return _lookAt; }
    void setLookAt(QVector3D pt, bool fromMouse);

    inline bool perspective() { return _perspective; }
    void setPerspective(bool val);

    void usePreset(preset val);

    inline direction dir() { return _dir; }
    void setDir(direction val);

    inline bool rightHanded() { return _rightHanded; }
    void setRightHanded(bool val);

    inline bool showAxes() { return _showAxes; }
    void setShowAxes(bool val, bool fromMouse);

public slots:
    void initializeDispObject(DispObject *obj);

signals:
    void inclinationChanged(double val, bool fromMouse);
    void azimuthChanged(double val, bool fromMouse);
    void fovChanged(double val, bool fromMouse);
    void rollChanged(double val, bool fromMouse);
    void zoomChanged(double val, bool fromMouse);
    void lookAtChanged(QVector3D pt, bool fromMouse);
    void perspectiveChanged(bool val);
    void fixedChanged(bool val, preset view);
    void dirChanged(direction val);
    void rightHandedChanged(bool val);
    void showAxesChanged(bool val, bool fromMouse);

    void singlePatchSelected(bool val);

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
    void drawAxes();
    void matrix(QMatrix4x4 *);
    void axesMatrix(QMatrix4x4 *);
    void multiplyDir(QMatrix4x4 *);

    QOpenGLShaderProgram vcProgram;
    QOpenGLShaderProgram ccProgram;
    QOpenGLShaderProgram lnProgram;
    QOpenGLBuffer auxBuffer;
    QOpenGLBuffer axesBuffer;
    QOpenGLBuffer auxCBuffer;

    ObjectSet *objectSet;
    DispObject *selectedObject;

    bool shiftPressed;
    bool ctrlPressed;
    bool altPressed;

    double _inclination;
    double _azimuth;
    double _roll;
    double _fov;
    double _zoom;
    QVector3D _lookAt;
    bool _perspective;
    bool _fixed;
    direction _dir;
    bool _rightHanded;
    bool _showAxes;
    float _diameter;

    bool cameraTracking;
    QPoint mouseOrig;
    double mouseOrigInclination, mouseOrigAzimuth, mouseOrigRoll;
    QVector3D mouseOrigLookAt;

    double orthoOrigFov;

    double fixedOrigInclination, fixedOrigAzimuth, fixedOrigRoll, fixedOrigFov, fixedOrigZoom;
    bool fixedOrigPerspective;
};

#endif /* _GLWIDGET_H_ */
