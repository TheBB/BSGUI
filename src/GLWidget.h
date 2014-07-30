#include <mutex>
#include <unordered_map>

#include <QVector3D>
#include <QVector4D>

#include <QGLWidget>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QSize>
#include <QWheelEvent>

#include "ObjectSet.h"
#include "DisplayObject.h"

#ifndef _GLWIDGET_H_
#define _GLWIDGET_H_

#define MAX_FOV 135.0
#define MAX_ZOOM 3.0

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

    inline double inclination() { return _inclination; }
    void setInclination(double val);

    inline double azimuth() { return _azimuth; }
    void setAzimuth(double val);

    inline double roll() { return _roll; }
    void setRoll(double roll);

    inline double fov() { return _fov; }
    void setFov(double val);

    inline double zoom() { return _zoom; }
    void setZoom(double val);

    inline QVector3D lookAt() { return _lookAt; }
    void setLookAt(QVector3D pt, bool fromMouse);

    inline bool perspective() { return _perspective; }
    void setPerspective(bool val);

    inline bool fixed() { return _fixed; }

    void usePreset(preset val);

    inline direction dir() { return _dir; }
    void setDir(direction val);

    inline bool rightHanded() { return _rightHanded; }
    void setRightHanded(bool val);

    inline bool showAxes() { return _showAxes; }
    void setShowAxes(bool val);

    inline bool showPoints() { return _showPoints; }
    void setShowPoints(bool val);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

public slots:
    void initializeDispObject(DisplayObject *obj);

signals:
    void inclinationChanged(double val);
    void azimuthChanged(double val);
    void fovChanged(double val);
    void rollChanged(double val);
    void zoomChanged(double val);
    void lookAtChanged(QVector3D pt, bool fromMouse);
    void perspectiveChanged(bool val);
    void fixedChanged(bool val, preset view);
    void dirChanged(direction val);
    void rightHandedChanged(bool val);
    void showAxesChanged(bool val);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    std::set<uint> paintGLPicks(int x, int y, int w, int h);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    void drawAxes();
    void drawSelection();
    void matrix(QMatrix4x4 *);
    void axesMatrix(QMatrix4x4 *);
    void multiplyDir(QMatrix4x4 *);

    QOpenGLShaderProgram vcProgram, ccProgram;
    QOpenGLBuffer auxBuffer, axesBuffer, selectionBuffer, auxCBuffer;

    ObjectSet *objectSet;

    bool shiftPressed, ctrlPressed, altPressed;

    double _inclination, _azimuth, _roll, _fov, _zoom, _diameter;
    bool _perspective, _fixed, _rightHanded, _showAxes, _showPoints;
    QVector3D _lookAt;
    direction _dir;

    bool selectTracking;
    QPoint selectOrig, selectTo;

    bool cameraTracking;
    QPoint mouseOrig;
    double mouseOrigInclination, mouseOrigAzimuth, mouseOrigRoll;
    QVector3D mouseOrigLookAt;

    double orthoOrigFov;

    double fixedOrigInclination, fixedOrigAzimuth, fixedOrigRoll, fixedOrigFov, fixedOrigZoom;
    bool fixedOrigPerspective;
};

#endif /* _GLWIDGET_H_ */
