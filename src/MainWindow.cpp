#include <sstream>
#include <thread>
#include <QVector3D>

#include "main.h"

#include "MainWindow.h"


void makeCubes(ObjectSet *objectSet, GLWidget *glWidget)
{
    objectSet->addPatchesFromFile("NREL_wing_mesh_3D.g2");

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

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

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
    case Qt::Key_QuoteLeft: glWidget->usePreset(VIEW_FREE); return true;
    case Qt::Key_1: glWidget->usePreset(VIEW_TOP); return true;
    case Qt::Key_2: glWidget->usePreset(VIEW_BOTTOM); return true;
    case Qt::Key_3: glWidget->usePreset(VIEW_LEFT); return true;
    case Qt::Key_4: glWidget->usePreset(VIEW_RIGHT); return true;
    case Qt::Key_5: glWidget->usePreset(VIEW_FRONT); return true;
    case Qt::Key_6: glWidget->usePreset(VIEW_BACK); return true;
    case Qt::Key_A: glWidget->setShowAxes(!glWidget->showAxes(), true); return true;
    case Qt::Key_C: glWidget->centerOnSelected(); return true;

    case Qt::Key_E:
        if (e->modifiers().testFlag(Qt::ControlModifier))
            objectSet->setSelectionMode(SM_EDGE, true);
        return true;

    case Qt::Key_F:
        if (e->modifiers().testFlag(Qt::ControlModifier))
            objectSet->setSelectionMode(SM_FACE, true);
        return true;

    case Qt::Key_P:
        if (e->modifiers().testFlag(Qt::ControlModifier))
            objectSet->setSelectionMode(SM_PATCH, true);
        else if (!glWidget->fixed())
            glWidget->setPerspective(!glWidget->perspective());
        return true;

    case Qt::Key_V:
        if (e->modifiers().testFlag(Qt::ControlModifier))
            objectSet->setSelectionMode(SM_POINT, true);
        return true;

    case Qt::Key_H:
        if (e->modifiers().testFlag(Qt::ControlModifier))
            objectSet->showAllSelectedPatches(false);
        else
            objectSet->showSelected(false);
        return true;

    case Qt::Key_S:
        if (e->modifiers().testFlag(Qt::ControlModifier) && e->modifiers().testFlag(Qt::ShiftModifier))
            objectSet->showAll();
        else if (e->modifiers().testFlag(Qt::ControlModifier))
            objectSet->showAllSelectedPatches(true);
        else
            objectSet->showSelected(true);
        return true;

    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt: glWidget->keyPressEvent(e); return true;
    default: return false;
    }
}
