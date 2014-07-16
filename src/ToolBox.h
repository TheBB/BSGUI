#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSize>
#include <QSlider>
#include <QWidget>

#include "GLWidget.h"

#ifndef _TOOLBOX_H_
#define _TOOLBOX_H_

class CameraPanel : public QWidget
{
    Q_OBJECT

public:
    CameraPanel(GLWidget *glWidget, QWidget *parent = NULL, Qt::WindowFlags flags = 0);
    ~CameraPanel() { }

    QSize sizeHint() const { return QSize(300, 100); }

public slots:
    void inclinationChanged(double val, bool fromMouse);
    void azimuthChanged(double val, bool fromMouse);
    void fovChanged(double val, bool fromMouse);
    void zoomChanged(double val, bool fromMouse);
    void lookAtChanged(QVector3D pt, bool fromMouse);
    void updateLookAt(double t);

private:
    GLWidget *glWidget;

    QLabel *inclinationLabel, *azimuthLabel, *fovLabel, *zoomLabel;
    QSlider *inclinationSlider, *azimuthSlider, *fovSlider, *zoomSlider;
    QDoubleSpinBox *lookAtX, *lookAtY, *lookAtZ;
};


class TreePanel : public QWidget
{
    Q_OBJECT

public:
    TreePanel(QWidget *parent = NULL, Qt::WindowFlags flags = 0);
    ~TreePanel() { }

    QSize sizeHint() const { return QSize(300, 100); }
};


class ToolBox : public QDockWidget
{
    Q_OBJECT

public:
    ToolBox(GLWidget *glWidget, const QString &title, QWidget *parent = NULL, Qt::WindowFlags flags = 0);
    ~ToolBox() { }
};

#endif /* _TOOLBOX_H_ */
