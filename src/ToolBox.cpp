#include <cmath>

#include <QGridLayout>
#include <QToolBox>
#include <QTreeView>
#include <QVBoxLayout>

#include "ToolBox.h"

#define INCSLIDER_FACTOR 10
#define AZMSLIDER_FACTOR 10
#define FOVSLIDER_FACTOR 200
#define ZOOMSLIDER_FACTOR 500


TreePanel::TreePanel(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    QVBoxLayout *layout = new QVBoxLayout();

    QTreeView *treeView = new QTreeView();
    layout->addWidget(treeView);

    setLayout(layout);
}


CameraPanel::CameraPanel(GLWidget *glWidget, QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    QGridLayout *layout = new QGridLayout();

    int row = 0;



    inclinationLabel = new QLabel();
    layout->addWidget(new QLabel("Inclination"), row, 0, 1, 1, Qt::AlignLeft);
    layout->addWidget(inclinationLabel, row, 1, 1, 1, Qt::AlignRight);

    inclinationSlider = new QSlider(Qt::Horizontal);
    inclinationSlider->setMinimum(-90 * INCSLIDER_FACTOR);
    inclinationSlider->setMaximum(90 * INCSLIDER_FACTOR);
    layout->addWidget(inclinationSlider, row+1, 0, 1, 2);

    inclinationChanged(glWidget->inclination(), true);

    QObject::connect(glWidget, &GLWidget::inclinationChanged, this, &CameraPanel::inclinationChanged);
    QObject::connect(inclinationSlider, &QSlider::valueChanged,
                     [glWidget] (int val) {
                         glWidget->setInclination((float) val / INCSLIDER_FACTOR, false);
                         glWidget->update();
                     });

    row += 2;


    azimuthLabel = new QLabel();
    layout->addWidget(new QLabel("Azimuth"), row, 0, 1, 1, Qt::AlignLeft);
    layout->addWidget(azimuthLabel, row, 1, 1, 1, Qt::AlignRight);

    azimuthSlider = new QSlider(Qt::Horizontal);
    azimuthSlider->setMinimum(0);
    azimuthSlider->setMaximum(360 * AZMSLIDER_FACTOR);
    layout->addWidget(azimuthSlider, row+1, 0, 1, 2);

    azimuthChanged(glWidget->azimuth(), true);

    QObject::connect(glWidget, &GLWidget::azimuthChanged, this, &CameraPanel::azimuthChanged);
    QObject::connect(azimuthSlider, &QSlider::valueChanged,
                     [glWidget] (int val) {
                         glWidget->setAzimuth((float) val / AZMSLIDER_FACTOR, false);
                         glWidget->update();
                     });

    row += 2;


    fovLabel = new QLabel();
    layout->addWidget(new QLabel("Optical zoom"), row, 0, 1, 1, Qt::AlignLeft);
    layout->addWidget(fovLabel, row, 1, 1, 1, Qt::AlignRight);

    fovSlider = new QSlider(Qt::Horizontal);
    fovSlider->setMinimum(-5 * FOVSLIDER_FACTOR);
    fovSlider->setMaximum(0);
    layout->addWidget(fovSlider, row+1, 0, 1, 2);

    fovChanged(glWidget->fov(), true);

    QObject::connect(glWidget, &GLWidget::fovChanged, this, &CameraPanel::fovChanged);
    QObject::connect(fovSlider, &QSlider::valueChanged,
                     [glWidget] (int val) {
                         glWidget->setFov(exp((float) val / FOVSLIDER_FACTOR) * MAX_FOV, false);
                         glWidget->update();
                     });

    row += 2;

    zoomLabel = new QLabel();
    layout->addWidget(new QLabel("Physical zoom"), row, 0, 1, 1, Qt::AlignLeft);
    layout->addWidget(zoomLabel, row, 1, 1, 1, Qt::AlignRight);

    zoomSlider = new QSlider(Qt::Horizontal);
    zoomSlider->setMinimum(-1 * ZOOMSLIDER_FACTOR);
    zoomSlider->setMaximum(2 * ZOOMSLIDER_FACTOR);
    layout->addWidget(zoomSlider, row+1, 0, 1, 2);

    zoomChanged(glWidget->zoom(), true);

    QObject::connect(glWidget, &GLWidget::zoomChanged, this, &CameraPanel::zoomChanged);
    QObject::connect(zoomSlider, &QSlider::valueChanged,
                     [glWidget] (int val) {
                         glWidget->setZoom((float) val / ZOOMSLIDER_FACTOR, false);
                         glWidget->update();
                     });

    row += 2;


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
    {
        bool prev = inclinationSlider->blockSignals(true);
        inclinationSlider->setValue((int) round(val * INCSLIDER_FACTOR));
        inclinationSlider->blockSignals(prev);
    }
}


void CameraPanel::azimuthChanged(double val, bool fromMouse)
{
    azimuthLabel->setText(QString::number(val, 'f', 1));
    if (fromMouse)
    {
        bool prev = azimuthSlider->blockSignals(true);
        azimuthSlider->setValue((int) round(val * AZMSLIDER_FACTOR));
        azimuthSlider->blockSignals(prev);
    }
}


void CameraPanel::fovChanged(double val, bool fromMouse)
{
    fovLabel->setText(QString::number(val, 'f', 1));
    if (fromMouse)
    {
        bool prev = fovSlider->blockSignals(true);
        fovSlider->setValue((int) round(FOVSLIDER_FACTOR * log(val / MAX_FOV)));
        fovSlider->blockSignals(prev);
    }
}


void CameraPanel::zoomChanged(double val, bool fromMouse)
{
    zoomLabel->setText(QString::number(val, 'f', 2));
    if (fromMouse)
    {
        bool prev = zoomSlider->blockSignals(true);
        zoomSlider->setValue((int) round(ZOOMSLIDER_FACTOR * val));
        zoomSlider->blockSignals(prev);
    }
}


ToolBox::ToolBox(GLWidget *glWidget, const QString &title, QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(title, parent, flags)
{
    QToolBox *toolBox = new QToolBox();

    TreePanel *treePanel = new TreePanel();
    toolBox->addItem(treePanel, "Objects");

    CameraPanel *cameraPanel = new CameraPanel(glWidget);
    toolBox->addItem(cameraPanel, "Camera");

    toolBox->setCurrentIndex(1);

    setWidget(toolBox);
}
