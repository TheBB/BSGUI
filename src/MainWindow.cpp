#include <sstream>

#include <QVector3D>

#include "DispObject.h"
#include "GLWidget.h"
#include "ToolBox.h"
#include "main.h"

#include "MainWindow.h"


MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    std::ostringstream ss;
    ss << "SINTEF BSGUI "
       << BSGUI_VERSION_MAJOR << "."
       << BSGUI_VERSION_MINOR << "."
       << BSGUI_VERSION_PATCH;
    setWindowTitle(QString(ss.str().c_str()));

    objectSet = new ObjectSet();

    GLWidget *glWidget = new GLWidget(objectSet, this);
    setCentralWidget(glWidget);
    glWidget->setFocus();

    ToolBox *toolbox = new ToolBox(glWidget, objectSet, "Toolbox", this);
    toolbox->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, toolbox);

    std::vector<QVector3D> centers = {
        QVector3D(0, 0, 1),
        QVector3D(-3, 0, 0),
        QVector3D(0, -3, 0),
        QVector3D(6, 0, 0),
        QVector3D(0, 4, 0),
    };

    for (auto c : centers)
        objectSet->addCubeFromCenter(c);
}


MainWindow::~MainWindow()
{
    delete objectSet;
}
