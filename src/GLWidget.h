/*
 * Copyright (C) 2014 SINTEF ICT,
 * Applied Mathematics, Norway.
 *
 * Contact information:
 * E-mail: eivind.fonn@sintef.no
 * SINTEF ICT, Department of Applied Mathematics,
 * P.O. Box 4760 Sluppen,
 * 7045 Trondheim, Norway.
 *
 * This file is part of BSGUI.
 *
 * BSGUI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * BSGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with GoTools. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In accordance with Section 7(b) of the GNU Affero General Public
 * License, a covered work must retain the producer line in every data
 * file that is created or manipulated using GoTools.
 *
 * Other Usage
 * You can be released from the requirements of the license by purchasing
 * a commercial license. Buying such a license is mandatory as soon as you
 * develop commercial activities involving the GoTools library without
 * disclosing the source code of your own applications.
 *
 * This file may be used in accordance with the terms contained in a
 * written agreement between you and SINTEF ICT.
 */

#include <mutex>
#include <unordered_map>

#include <QVector3D>
#include <QVector4D>

#include <QGLFormat>
#include <QGLWidget>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
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
    GLWidget(QGLFormat fmt, ObjectSet *oSet, QWidget *parent = NULL);
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
    std::set<std::pair<uint,uint>> paintGLPicks(int x, int y, int w, int h);

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    void initializeAux();

    void drawAxes();
    void drawSelection();
    void matrix(QMatrix4x4 *);
    void axesMatrix(QMatrix4x4 *);
    void multiplyDir(QMatrix4x4 *);

    QOpenGLShaderProgram vcProgram, ccProgram;
    QOpenGLBuffer auxBuffer, axesBuffer, selectionBuffer, auxCBuffer;
    QOpenGLVertexArrayObject vao;

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
