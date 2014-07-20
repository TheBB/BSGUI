#include <sstream>
#include <thread>

#include <QVector3D>

#include "DispObject.h"
#include "GLWidget.h"
#include "ToolBox.h"
#include "main.h"

#include "MainWindow.h"


void makeCubes(ObjectSet *objectSet, GLWidget *glWidget)
{
    std::vector<QVector3D> centers = {
        QVector3D(0, 0, 1),
        QVector3D(-3, 0, 0),
        QVector3D(0, -3, 0),
        QVector3D(6, 0, 0),
        QVector3D(0, 4, 0),
    };


    for (auto c : centers)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        objectSet->addCubeFromCenter(c);
    }

    glWidget->centerOnSelected();
}


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

    std::thread *thread = new std::thread(makeCubes, objectSet, glWidget);
}


MainWindow::~MainWindow()
{
    delete objectSet;
}
