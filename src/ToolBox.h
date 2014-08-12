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
