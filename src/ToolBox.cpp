#include <cmath>

#include <QGridLayout>
#include <QToolBox>
#include <QTreeView>
#include <QVBoxLayout>

#include "ToolBox.h"


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
    inclinationSlider->setMinimum(-10 * 90);
    inclinationSlider->setMaximum(10 * 90);
    layout->addWidget(inclinationSlider, row+1, 0, 1, 2);

    inclinationChanged(glWidget->inclination(), true);

    QObject::connect(glWidget, SIGNAL(inclinationChanged(double, bool)),
                     this, SLOT(inclinationChanged(double, bool)));
    QObject::connect(inclinationSlider, &QSlider::valueChanged,
                     [glWidget] (int val) {
                         glWidget->setInclination((float) val / 10.0, false);
                         glWidget->update();
                     });

    row += 2;


    azimuthLabel = new QLabel();
    layout->addWidget(new QLabel("Azimuth"), row, 0, 1, 1, Qt::AlignLeft);
    layout->addWidget(azimuthLabel, row, 1, 1, 1, Qt::AlignRight);

    azimuthSlider = new QSlider(Qt::Horizontal);
    azimuthSlider->setMinimum(0);
    azimuthSlider->setMaximum(10 * 360);
    layout->addWidget(azimuthSlider, row+1, 0, 1, 2);

    azimuthChanged(glWidget->azimuth(), true);

    QObject::connect(glWidget, SIGNAL(azimuthChanged(double, bool)),
                     this, SLOT(azimuthChanged(double,bool)));
    QObject::connect(azimuthSlider, &QSlider::valueChanged,
                     [glWidget] (int val) {
                         glWidget->setAzimuth((float) val / 10.0, false);
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
        inclinationSlider->setValue((int) round(val * 10));
        inclinationSlider->blockSignals(prev);
    }
}


void CameraPanel::azimuthChanged(double val, bool fromMouse)
{
    azimuthLabel->setText(QString::number(val, 'f', 1));
    if (fromMouse)
    {
        bool prev = azimuthSlider->blockSignals(true);
        azimuthSlider->setValue((int) round(val * 10));
        azimuthSlider->blockSignals(prev);
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
