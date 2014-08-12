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

#include <cmath>
#include <QAbstractItemModel>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QRadioButton>
#include <QTime>
#include <QTimer>
#include <QToolBox>
#include <QTreeView>
#include <QVBoxLayout>
#include "ObjectSet.h"
#include "ToolBox.h"
#define INCSLIDER_FACTOR 10
#define AZMSLIDER_FACTOR 10
#define ROLLSLIDER_FACTOR 10
#define FOVSLIDER_FACTOR 200
#define ZOOMSLIDER_FACTOR 500


TreePanel::TreePanel(GLWidget *glWidget, ObjectSet *objectSet, QWidget *filter,
                     QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    QVBoxLayout *layout = new QVBoxLayout();

    QTreeView *treeView = new QTreeView();
    treeView->installEventFilter(filter);
    layout->addWidget(treeView);
    treeView->setModel(objectSet);

    treeView->header()->setStretchLastSection(false);
    treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    treeView->resizeColumnToContents(1);
    treeView->resizeColumnToContents(2);


    QGroupBox *selModePanel = new QGroupBox("Selection mode");
    layout->addWidget(selModePanel);
    QGridLayout *selModeLayout = new QGridLayout();
    selModePanel->setLayout(selModeLayout);

    QRadioButton *patchesBtn = new QRadioButton("Patches");
    selModeLayout->addWidget(patchesBtn, 0, 0, 1, 1);
    patchesBtn->setChecked(objectSet->selectionMode() == SM_PATCH);

    QRadioButton *facesBtn = new QRadioButton("Faces");
    selModeLayout->addWidget(facesBtn, 0, 1, 1, 1);
    facesBtn->setChecked(objectSet->selectionMode() == SM_FACE);

    QRadioButton *edgesBtn = new QRadioButton("Edges");
    selModeLayout->addWidget(edgesBtn, 1, 0, 1, 1);
    edgesBtn->setChecked(objectSet->selectionMode() == SM_EDGE);

    QRadioButton *pointsBtn = new QRadioButton("Vertices");
    selModeLayout->addWidget(pointsBtn, 1, 1, 1, 1);
    pointsBtn->setChecked(objectSet->selectionMode() == SM_POINT);

    QObject::connect(patchesBtn, &QRadioButton::clicked,
                     [objectSet] (bool checked) { if (checked) objectSet->setSelectionMode(SM_PATCH); });
    QObject::connect(facesBtn, &QRadioButton::clicked,
                     [objectSet] (bool checked) { if (checked) objectSet->setSelectionMode(SM_FACE); });
    QObject::connect(edgesBtn, &QRadioButton::clicked,
                     [objectSet] (bool checked) { if (checked) objectSet->setSelectionMode(SM_EDGE); });
    QObject::connect(pointsBtn, &QRadioButton::clicked,
                     [objectSet] (bool checked) { if (checked) objectSet->setSelectionMode(SM_POINT); });
    QObject::connect(objectSet, &ObjectSet::selectionModeChanged,
                     [patchesBtn, facesBtn, edgesBtn, pointsBtn] (SelectionMode mode) {
                         patchesBtn->setChecked(mode == SM_PATCH);
                         facesBtn->setChecked(mode == SM_FACE);
                         edgesBtn->setChecked(mode == SM_EDGE);
                         pointsBtn->setChecked(mode == SM_POINT);
                     });

    setLayout(layout);
}


template <typename T, typename V>
void blockAndSet(T *obj, V val)
{
    bool prev = obj->blockSignals(true);
    obj->setValue(val);
    obj->blockSignals(prev);
}


void newLabelSet(QLabel **label, QGridLayout *layout, QString title, int row)
{
    *label = new QLabel();
    layout->addWidget(new QLabel(title), row, 0, 1, 2, Qt::AlignLeft);
    layout->addWidget(*label, row, 2, 1, 1, Qt::AlignRight);
}


void newSlider(QSlider **slider, QGridLayout *layout, int min, int max, int row)
{
    *slider = new QSlider(Qt::Horizontal);
    (*slider)->setMinimum(min);
    (*slider)->setMaximum(max);
    layout->addWidget(*slider, row, 0, 1, 3);
}


void newUpRadioButton(QString title, QGridLayout *layout, int row, int col, GLWidget *glWidget, direction dir)
{
    QRadioButton *btn = new QRadioButton(title);
    layout->addWidget(btn, row, col, 1, 1);
    btn->setChecked(glWidget->dir() == dir);
    QObject::connect(btn, &QRadioButton::clicked,
                     [glWidget, dir] (bool checked) { if (checked) glWidget->setDir(dir); });
}


void newPresetsRadioButton(QString title, QGridLayout *layout, int row, int col,
                           GLWidget *glWidget, preset val, bool checked=false)
{
    QRadioButton *btn = new QRadioButton(title);
    btn->setChecked(checked);
    layout->addWidget(btn, row, col, 1, 1);
    QObject::connect(btn, &QRadioButton::clicked,
                     [glWidget, val] (bool checked) { if (checked) glWidget->usePreset(val); });
    QObject::connect(glWidget, &GLWidget::fixedChanged,
                     [btn, val] (bool fixed, preset view) { btn->setChecked(view == val); });
}


CameraPanel::CameraPanel(GLWidget *glWidget, ObjectSet *objectSet,
                         QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , glWidget(glWidget)
{
    QGridLayout *layout = new QGridLayout();

    int row = 0;


    newLabelSet(&inclinationLabel, layout, "Inclination", row);
    newSlider(&inclinationSlider, layout, -90*INCSLIDER_FACTOR, 90*INCSLIDER_FACTOR, row+1);

    QObject::connect(glWidget, &GLWidget::inclinationChanged, this, &CameraPanel::inclinationChanged);
    QObject::connect(inclinationSlider, &QSlider::valueChanged,
                     [glWidget] (int val) {
                         glWidget->setInclination((float) val / INCSLIDER_FACTOR);
                     });

    row += 2;


    newLabelSet(&azimuthLabel, layout, "Azimuth", row);
    newSlider(&azimuthSlider, layout, 0, 360*AZMSLIDER_FACTOR, row+1);

    QObject::connect(glWidget, &GLWidget::azimuthChanged, this, &CameraPanel::azimuthChanged);
    QObject::connect(azimuthSlider, &QSlider::valueChanged,
                     [glWidget] (int val) {
                         glWidget->setAzimuth((float) val / AZMSLIDER_FACTOR);
                     });

    row += 2;


    newLabelSet(&rollLabel, layout, "Roll", row);
    newSlider(&rollSlider, layout, 0, 360*ROLLSLIDER_FACTOR, row+1);

    QObject::connect(glWidget, &GLWidget::rollChanged, this, &CameraPanel::rollChanged);
    QObject::connect(rollSlider, &QSlider::valueChanged,
                     [glWidget] (int val) {
                         glWidget->setRoll((float) val / ROLLSLIDER_FACTOR);
                     });

    row += 2;


    newLabelSet(&fovLabel, layout, "Optical zoom", row);
    newSlider(&fovSlider, layout, -5*FOVSLIDER_FACTOR, 0, row+1);

    QObject::connect(glWidget, &GLWidget::fovChanged, this, &CameraPanel::fovChanged);
    QObject::connect(fovSlider, &QSlider::valueChanged,
                     [glWidget] (int val) {
                         glWidget->setFov(exp((float) val / FOVSLIDER_FACTOR) * MAX_FOV);
                     });

    row += 2;


    newLabelSet(&zoomLabel, layout, "Physical zoom", row);
    newSlider(&zoomSlider, layout, 0, MAX_ZOOM * ZOOMSLIDER_FACTOR, row+1);

    QObject::connect(glWidget, &GLWidget::zoomChanged, this, &CameraPanel::zoomChanged);
    QObject::connect(zoomSlider, &QSlider::valueChanged,
                     [glWidget] (int val) {
                         glWidget->setZoom((float) val / ZOOMSLIDER_FACTOR);
                     });

    row += 2;


    QGroupBox *lookAtPanel = new QGroupBox("Look at");
    layout->addWidget(lookAtPanel, row, 0, 1, 3);
    QHBoxLayout *lookAtLayout = new QHBoxLayout();
    lookAtPanel->setLayout(lookAtLayout);

    std::vector<QDoubleSpinBox **> lookAts = {&lookAtX, &lookAtY, &lookAtZ};
    for (auto lookAt : lookAts)
    {
        *lookAt = new QDoubleSpinBox();
        lookAtLayout->addWidget(*lookAt);
        (*lookAt)->setMinimum(-std::numeric_limits<double>::infinity());
        (*lookAt)->setMaximum(std::numeric_limits<double>::infinity());
        QObject::connect(*lookAt, SIGNAL(valueChanged(double)), this, SLOT(updateLookAt(double)));
    }

    QObject::connect(glWidget, &GLWidget::lookAtChanged, this, &CameraPanel::lookAtChanged);

    row++;


    QPushButton *centerBtn = new QPushButton("Center camera on scene");
    layout->addWidget(centerBtn, row, 0, 1, 3);

    QObject::connect(centerBtn, &QPushButton::clicked,
                     [glWidget] (bool val) { glWidget->centerOnSelected(); });
    QObject::connect(objectSet, &ObjectSet::selectionChanged,
                     [centerBtn, objectSet] () {
                         centerBtn->setText(objectSet->hasSelection()
                                            ? "Center camera on selection"
                                            : "Center camera on scene");
                     });

    row++;


    QGroupBox *projPanel = new QGroupBox("Projection");
    layout->addWidget(projPanel, row, 0, 1, 3);
    QHBoxLayout *projLayout = new QHBoxLayout();
    projPanel->setLayout(projLayout);

    perspectiveBtn = new QRadioButton("Perspective");
    projLayout->addWidget(perspectiveBtn);
    perspectiveBtn->setChecked(glWidget->perspective());

    orthographicBtn = new QRadioButton("Orthographic");
    projLayout->addWidget(orthographicBtn);
    orthographicBtn->setChecked(!glWidget->perspective());

    QObject::connect(glWidget, &GLWidget::perspectiveChanged, this, &CameraPanel::perspectiveChanged);
    QObject::connect(perspectiveBtn, &QRadioButton::clicked,
                     [glWidget] (bool checked) { glWidget->setPerspective(checked); });
    QObject::connect(orthographicBtn, &QRadioButton::clicked,
                     [glWidget] (bool checked) { glWidget->setPerspective(!checked); });

    row++;


    QGroupBox *presetsPanel = new QGroupBox("Presets");
    layout->addWidget(presetsPanel, row, 0, 1, 3);
    QGridLayout *presetsLayout = new QGridLayout();
    presetsPanel->setLayout(presetsLayout);

    newPresetsRadioButton("Top", presetsLayout, 0, 0, glWidget, VIEW_TOP);
    newPresetsRadioButton("Left", presetsLayout, 0, 1, glWidget, VIEW_LEFT);
    newPresetsRadioButton("Front", presetsLayout, 0, 2, glWidget, VIEW_FRONT);
    newPresetsRadioButton("Bottom", presetsLayout, 1, 0, glWidget, VIEW_BOTTOM);
    newPresetsRadioButton("Right", presetsLayout, 1, 1, glWidget, VIEW_RIGHT);
    newPresetsRadioButton("Back", presetsLayout, 1, 2, glWidget, VIEW_BACK);
    newPresetsRadioButton("Free", presetsLayout, 2, 1, glWidget, VIEW_FREE, true);

    row++;


    QGroupBox *upPanel = new QGroupBox("Up is...");
    layout->addWidget(upPanel, row, 0, 1, 3);
    QGridLayout *upLayout = new QGridLayout();
    upPanel->setLayout(upLayout);

    newUpRadioButton("Positive X", upLayout, 0, 0, glWidget, POSX);
    newUpRadioButton("Negative X", upLayout, 0, 1, glWidget, NEGX);
    newUpRadioButton("Positive Y", upLayout, 1, 0, glWidget, POSY);
    newUpRadioButton("Negative Y", upLayout, 1, 1, glWidget, NEGY);
    newUpRadioButton("Positive Z", upLayout, 2, 0, glWidget, POSZ);
    newUpRadioButton("Negative Z", upLayout, 2, 1, glWidget, NEGZ);

    row++;


    QGroupBox *coordPanel = new QGroupBox("Coordinate system is...");
    layout->addWidget(coordPanel, row, 0, 1, 3);
    QHBoxLayout *coordLayout = new QHBoxLayout();
    coordPanel->setLayout(coordLayout);

    QRadioButton *rightHanded = new QRadioButton("Right handed");
    coordLayout->addWidget(rightHanded);
    rightHanded->setChecked(glWidget->rightHanded());

    QRadioButton *leftHanded = new QRadioButton("Left handed");
    coordLayout->addWidget(leftHanded);
    leftHanded->setChecked(!glWidget->rightHanded());

    QObject::connect(rightHanded, &QRadioButton::clicked,
                     [glWidget] (bool checked) { glWidget->setRightHanded(checked); });
    QObject::connect(leftHanded, &QRadioButton::clicked,
                     [glWidget] (bool checked) { glWidget->setRightHanded(!checked); });

    row++;


    showAxes = new QCheckBox("Show axes");
    layout->addWidget(showAxes, row, 0, 1, 3);
    showAxes->setChecked(glWidget->showAxes());

    QObject::connect(showAxes, &QCheckBox::clicked,
                     [glWidget] (bool checked) { glWidget->setShowAxes(checked); });
    QObject::connect(glWidget, &GLWidget::showAxesChanged,
                     [this] (bool val) { showAxes->setChecked(val); });

    row++;


    showPoints = new QCheckBox("Show vertices");
    layout->addWidget(showPoints, row, 0, 1, 3);
    showPoints->setChecked(glWidget->showPoints());

    QObject::connect(showPoints, &QCheckBox::toggled,
                     [glWidget] (bool checked) { glWidget->setShowPoints(checked); });
    
    row++;


    QObject::connect(glWidget, &GLWidget::fixedChanged, this, &CameraPanel::fixedChanged);


    inclinationChanged(glWidget->inclination());
    azimuthChanged(glWidget->azimuth());
    rollChanged(glWidget->roll());
    fovChanged(glWidget->fov());
    zoomChanged(glWidget->zoom());
    lookAtChanged(glWidget->lookAt(), true);


    QWidget *fill = new QWidget();
    layout->addWidget(fill, row, 0, 1, -1);

    layout->setRowStretch(row, 1);
    layout->setColumnStretch(1, 1);
    setLayout(layout);
}


void CameraPanel::inclinationChanged(double val)
{
    inclinationLabel->setText(QString::number(val, 'f', 1));
    inclinationSlider->setValue((int) round(val * INCSLIDER_FACTOR));
}


void CameraPanel::azimuthChanged(double val)
{
    azimuthLabel->setText(QString::number(val, 'f', 1));
    azimuthSlider->setValue((int) round(val * AZMSLIDER_FACTOR));
}


void CameraPanel::rollChanged(double val)
{
    rollLabel->setText(QString::number(val, 'f', 1));
    rollSlider->setValue((int) round(val * ROLLSLIDER_FACTOR));
}


void CameraPanel::fovChanged(double val)
{
    fovLabel->setText(QString::number(val, 'f', 1));
    fovSlider->setValue((int) round(FOVSLIDER_FACTOR * log(val / MAX_FOV)));
}


void CameraPanel::zoomChanged(double val)
{
    zoomLabel->setText(QString::number(val, 'f', 2));
    zoomSlider->setValue((int) round(ZOOMSLIDER_FACTOR * val));
}


void CameraPanel::lookAtChanged(QVector3D pt, bool fromMouse)
{
    if (fromMouse)
    {
        blockAndSet<QDoubleSpinBox, double>(lookAtX, pt.x());
        blockAndSet<QDoubleSpinBox, double>(lookAtY, pt.y());
        blockAndSet<QDoubleSpinBox, double>(lookAtZ, pt.z());
    }
}


void CameraPanel::updateLookAt(double t)
{
    glWidget->setLookAt(QVector3D(lookAtX->value(), lookAtY->value(), lookAtZ->value()), false);
}


void CameraPanel::perspectiveChanged(bool val)
{
    zoomSlider->setEnabled(val);
    perspectiveBtn->setChecked(val);
    orthographicBtn->setChecked(!val);
}


void CameraPanel::fixedChanged(bool val)
{
    inclinationSlider->setEnabled(!val);
    azimuthSlider->setEnabled(!val);
    perspectiveBtn->setEnabled(!val);
    orthographicBtn->setEnabled(!val);
}


ToolBox::ToolBox(GLWidget *glWidget, ObjectSet *objectSet,
                 const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(title, parent, flags)
{
    installEventFilter(parent);
    QToolBox *toolBox = new QToolBox();

    qRegisterMetaType<QVecInt>("QVecInt");
    TreePanel *treePanel = new TreePanel(glWidget, objectSet, parent, NULL);
    toolBox->addItem(treePanel, "Objects");

    CameraPanel *cameraPanel = new CameraPanel(glWidget, objectSet, NULL);
    toolBox->addItem(cameraPanel, "Camera");

    toolBox->setCurrentIndex(0);

    setWidget(toolBox);
}
