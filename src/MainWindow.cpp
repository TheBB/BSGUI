#include <sstream>

#include "ToolBox.h"
#include "GLWidget.h"
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

    GLWidget *glWidget = new GLWidget(this);
    setCentralWidget(glWidget);
    glWidget->setFocus();

    ToolBox *toolbox = new ToolBox(glWidget, "Toolbox", this);
    toolbox->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, toolbox);
}
