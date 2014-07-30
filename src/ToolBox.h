#include <QCheckBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSize>
#include <QSlider>
#include <QWidget>

#include "GLWidget.h"
#include "ObjectSet.h"

#ifndef _TOOLBOX_H_
#define _TOOLBOX_H_

typedef QVector<int> QVecInt;

class CameraPanel : public QWidget
{
    Q_OBJECT

public:
    CameraPanel(GLWidget *glWidget, ObjectSet *objectSet,
                QWidget *parent = NULL, Qt::WindowFlags flags = 0);
    ~CameraPanel() { }

    QSize sizeHint() const { return QSize(300, 100); }

public slots:
    void inclinationChanged(double val);
    void azimuthChanged(double val);
    void rollChanged(double val);
    void fovChanged(double val);
    void zoomChanged(double val);
    void lookAtChanged(QVector3D pt, bool fromMouse);
    void updateLookAt(double t);
    void perspectiveChanged(bool val);
    void fixedChanged(bool val);

private:
    GLWidget *glWidget;

    QLabel *inclinationLabel, *azimuthLabel, *fovLabel, *zoomLabel, *rollLabel;
    QSlider *inclinationSlider, *azimuthSlider, *fovSlider, *zoomSlider, *rollSlider;
    QDoubleSpinBox *lookAtX, *lookAtY, *lookAtZ;

    QRadioButton *perspectiveBtn, *orthographicBtn;
    QCheckBox *showAxes, *showPoints;
};


class TreePanel : public QWidget
{
    Q_OBJECT

public:
    TreePanel(GLWidget *glWidget, ObjectSet *objectSet, QWidget *filter,
              QWidget *parent = NULL, Qt::WindowFlags flags = 0);
    ~TreePanel() { }

    QSize sizeHint() const { return QSize(300, 100); }
};


class ToolBox : public QDockWidget
{
    Q_OBJECT

public:
    ToolBox(GLWidget *glWidget, ObjectSet *objectSet,
            const QString &title, QWidget *parent = NULL, Qt::WindowFlags flags = 0);
    ~ToolBox() { }
};

#endif /* _TOOLBOX_H_ */
