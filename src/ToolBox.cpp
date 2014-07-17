#include <cmath>

#include <QAbstractItemModel>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
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


TreePanel::TreePanel(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    QVBoxLayout *layout = new QVBoxLayout();

    QTreeView *treeView = new QTreeView();
    layout->addWidget(treeView);

    ObjectSet *objectSet = new ObjectSet(NULL);
    treeView->setModel(objectSet);

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
    QObject::connect(btn, &QRadioButton::toggled,
                     [glWidget, dir] (bool checked)
                     {
                         if (checked)
                         {
                             glWidget->setDir(dir);
                             glWidget->update();
                         }
                     });
}


void newPresetsRadioButton(QString title, QGridLayout *layout, int row, int col,
                           GLWidget *glWidget, preset val, bool checked=false)
{
    QRadioButton *btn = new QRadioButton(title);
    btn->setChecked(checked);
    layout->addWidget(btn, row, col, 1, 1);
    QObject::connect(btn, &QRadioButton::toggled,
                     [glWidget, val] (bool checked)
                     {
                         if (checked)
                         {
                             glWidget->usePreset(val);
                             glWidget->update();
                         }
                     });
    QObject::connect(glWidget, &GLWidget::fixedChanged,
                     [btn, val] (bool fixed, preset view) {btn->setChecked(view == val); });
}


CameraPanel::CameraPanel(GLWidget *glWidget, QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , glWidget(glWidget)
{
    QGridLayout *layout = new QGridLayout();

    int row = 0;


    newLabelSet(&inclinationLabel, layout, "Inclination", row);
    newSlider(&inclinationSlider, layout, -90*INCSLIDER_FACTOR, 90*INCSLIDER_FACTOR, row+1);

    QObject::connect(glWidget, &GLWidget::inclinationChanged, this, &CameraPanel::inclinationChanged);
    QObject::connect(inclinationSlider, &QSlider::valueChanged,
                     [glWidget] (int val)
                     {
                         glWidget->setInclination((float) val / INCSLIDER_FACTOR, false);
                         glWidget->update();
                     });

    row += 2;


    newLabelSet(&azimuthLabel, layout, "Azimuth", row);
    newSlider(&azimuthSlider, layout, 0, 360*AZMSLIDER_FACTOR, row+1);

    QObject::connect(glWidget, &GLWidget::azimuthChanged, this, &CameraPanel::azimuthChanged);
    QObject::connect(azimuthSlider, &QSlider::valueChanged,
                     [glWidget] (int val)
                     {
                         glWidget->setAzimuth((float) val / AZMSLIDER_FACTOR, false);
                         glWidget->update();
                     });

    row += 2;


    newLabelSet(&rollLabel, layout, "Roll", row);
    newSlider(&rollSlider, layout, 0, 360*ROLLSLIDER_FACTOR, row+1);

    QObject::connect(glWidget, &GLWidget::rollChanged, this, &CameraPanel::rollChanged);
    QObject::connect(rollSlider, &QSlider::valueChanged,
                     [glWidget] (int val)
                     {
                         glWidget->setRoll((float) val / ROLLSLIDER_FACTOR, false);
                         glWidget->update();
                     });

    row += 2;


    newLabelSet(&fovLabel, layout, "Optical zoom", row);
    newSlider(&fovSlider, layout, -5*FOVSLIDER_FACTOR, 0, row+1);

    QObject::connect(glWidget, &GLWidget::fovChanged, this, &CameraPanel::fovChanged);
    QObject::connect(fovSlider, &QSlider::valueChanged,
                     [glWidget] (int val)
                     {
                         glWidget->setFov(exp((float) val / FOVSLIDER_FACTOR) * MAX_FOV, false);
                         glWidget->update();
                     });

    row += 2;


    newLabelSet(&zoomLabel, layout, "Physical zoom", row);
    newSlider(&zoomSlider, layout, -2*ZOOMSLIDER_FACTOR, 2*ZOOMSLIDER_FACTOR, row+1);

    QObject::connect(glWidget, &GLWidget::zoomChanged, this, &CameraPanel::zoomChanged);
    QObject::connect(zoomSlider, &QSlider::valueChanged,
                     [glWidget] (int val)
                     {
                         glWidget->setZoom((float) val / ZOOMSLIDER_FACTOR, false);
                         glWidget->update();
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


    QPushButton *centerBtn = new QPushButton("Center on selected patch");
    centerBtn->setEnabled(false);
    layout->addWidget(centerBtn, row, 0, 1, 3);

    QObject::connect(glWidget, &GLWidget::singlePatchSelected,
                     [centerBtn] (bool val) { centerBtn->setEnabled(val); });
    QObject::connect(centerBtn, &QPushButton::clicked,
                     [glWidget] (bool val)
                     {
                         glWidget->centerOnSelected();
                         glWidget->update();
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
    QObject::connect(perspectiveBtn, &QRadioButton::toggled,
                     [glWidget] (bool checked)
                     {
                         glWidget->setPerspective(checked);
                         glWidget->update();
                     });

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

    QObject::connect(rightHanded, &QRadioButton::toggled,
                     [glWidget] (bool checked)
                     {
                         glWidget->setRightHanded(checked);
                         glWidget->update();
                     });

    row++;


    QObject::connect(glWidget, &GLWidget::fixedChanged, this, &CameraPanel::fixedChanged);


    inclinationChanged(glWidget->inclination(), true);
    azimuthChanged(glWidget->azimuth(), true);
    rollChanged(glWidget->roll(), true);
    fovChanged(glWidget->fov(), true);
    zoomChanged(glWidget->zoom(), true);
    lookAtChanged(glWidget->lookAt(), true);


    QWidget *fill = new QWidget();
    layout->addWidget(fill, row, 0, 1, -1);

    layout->setRowStretch(row, 1);
    layout->setColumnStretch(1, 1);
    setLayout(layout);
}


void CameraPanel::inclinationChanged(double val, bool fromMouse)
{
    inclinationLabel->setText(QString::number(val, 'f', 1));
    if (fromMouse)
        blockAndSet<QSlider, int>(inclinationSlider, (int) round(val * INCSLIDER_FACTOR));
}


void CameraPanel::azimuthChanged(double val, bool fromMouse)
{
    azimuthLabel->setText(QString::number(val, 'f', 1));
    if (fromMouse)
        blockAndSet<QSlider, int>(azimuthSlider, (int) round(val * AZMSLIDER_FACTOR));
}


void CameraPanel::rollChanged(double val, bool fromMouse)
{
    rollLabel->setText(QString::number(val, 'f', 1));
    if (fromMouse)
        blockAndSet<QSlider, int>(rollSlider, (int) round(val * ROLLSLIDER_FACTOR));
}


void CameraPanel::fovChanged(double val, bool fromMouse)
{
    fovLabel->setText(QString::number(val, 'f', 1));
    if (fromMouse)
        blockAndSet<QSlider, int>(fovSlider, (int) round(FOVSLIDER_FACTOR * log(val / MAX_FOV)));
}


void CameraPanel::zoomChanged(double val, bool fromMouse)
{
    zoomLabel->setText(QString::number(val, 'f', 2));
    if (fromMouse)
        blockAndSet<QSlider, int>(zoomSlider, (int) round(ZOOMSLIDER_FACTOR * val));
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
    glWidget->update();
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


ToolBox::ToolBox(GLWidget *glWidget, const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(title, parent, flags)
{
    QToolBox *toolBox = new QToolBox();

    TreePanel *treePanel = new TreePanel();
    toolBox->addItem(treePanel, "Objects");

    CameraPanel *cameraPanel = new CameraPanel(glWidget);
    toolBox->addItem(cameraPanel, "Camera");

    toolBox->setCurrentIndex(0);

    setWidget(toolBox);
}
