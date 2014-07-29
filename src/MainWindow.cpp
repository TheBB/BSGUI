#include <sstream>
#include <thread>

#include <QVector3D>

#include "main.h"

#include "MainWindow.h"


void makeCubes(ObjectSet *objectSet, GLWidget *glWidget)
{
    std::vector<QVector3D> centers = {
        QVector3D(0, 0, 1),
        // QVector3D(-3, 0, 0),
        // QVector3D(0, -3, 0),
        // QVector3D(6, 0, 0),
        // QVector3D(0, 4, 0),
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

    glWidget = new GLWidget(objectSet, this);
    setCentralWidget(glWidget);
    glWidget->setFocus();

    toolBox = new ToolBox(glWidget, objectSet, "Toolbox", this);
    toolBox->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, toolBox);

    std::thread *thread = new std::thread(makeCubes, objectSet, glWidget);
}


MainWindow::~MainWindow()
{
    delete objectSet;
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() != QEvent::KeyPress)
        return false;

    QKeyEvent *e = static_cast<QKeyEvent *>(event);
    switch (e->key())
    {
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt:
    case Qt::Key_C:
    case Qt::Key_A:
    case Qt::Key_P:
    case Qt::Key_QuoteLeft:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
        glWidget->keyPressEvent(e);
        return true;
    case Qt::Key_F:
        // objectSet->setSelectFaces(!objectSet->selectFaces(), true);
        return true;
    default:
        return false;
    }
}
