#include <sstream>
#include <thread>
#include <QMenuBar>
#include <QVector3D>

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

    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    _objectSet = new ObjectSet();

    _glWidget = new GLWidget(_objectSet, this);
    setCentralWidget(_glWidget);
    _glWidget->setFocus();

    _toolBox = new ToolBox(_glWidget, _objectSet, "Toolbox", this);
    _toolBox->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, _toolBox);

    _infoBox = new InfoBox(_objectSet, "Info", this);
    _infoBox->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, _infoBox);

    QMenu *windowsMenu = menuBar()->addMenu("Windows");
    _toolAct = windowsMenu->addAction("Toolbox");
    _infoAct = windowsMenu->addAction("Infobox");

    _toolAct->setCheckable(true);
    _infoAct->setCheckable(true);

    QObject::connect(_toolAct, &QAction::triggered,
                     [this] (bool checked) { this->_toolBox->setVisible(checked); });
    QObject::connect(_infoAct, &QAction::triggered,
                     [this] (bool checked) { this->_infoBox->setVisible(checked); });
}


MainWindow::~MainWindow()
{
    delete _objectSet;
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Show || event->type() == QEvent::Hide)
    {
        if (obj == _toolBox)
            _toolAct->setChecked(event->type() == QEvent::Show);

        if (obj == _infoBox)
            _infoAct->setChecked(event->type() == QEvent::Show);

        return false;
    }

    if (event->type() != QEvent::KeyPress)
        return false;

    QKeyEvent *e = static_cast<QKeyEvent *>(event);

    switch (e->key())
    {
    case Qt::Key_QuoteLeft: _glWidget->usePreset(VIEW_FREE); return true;
    case Qt::Key_1: _glWidget->usePreset(VIEW_TOP); return true;
    case Qt::Key_2: _glWidget->usePreset(VIEW_BOTTOM); return true;
    case Qt::Key_3: _glWidget->usePreset(VIEW_LEFT); return true;
    case Qt::Key_4: _glWidget->usePreset(VIEW_RIGHT); return true;
    case Qt::Key_5: _glWidget->usePreset(VIEW_FRONT); return true;
    case Qt::Key_6: _glWidget->usePreset(VIEW_BACK); return true;
    case Qt::Key_A: _glWidget->setShowAxes(!_glWidget->showAxes()); return true;
    case Qt::Key_C: _glWidget->centerOnSelected(); return true;

    case Qt::Key_E:
        if (e->modifiers().testFlag(Qt::ControlModifier))
            _objectSet->setSelectionMode(SM_EDGE);
        return true;

    case Qt::Key_F:
        if (e->modifiers().testFlag(Qt::ControlModifier))
            _objectSet->setSelectionMode(SM_FACE);
        return true;

    case Qt::Key_P:
        if (e->modifiers().testFlag(Qt::ControlModifier))
            _objectSet->setSelectionMode(SM_PATCH);
        else if (!_glWidget->fixed())
            _glWidget->setPerspective(!_glWidget->perspective());
        return true;

    case Qt::Key_V:
        if (e->modifiers().testFlag(Qt::ControlModifier))
            _objectSet->setSelectionMode(SM_POINT);
        return true;

    case Qt::Key_H:
        if (e->modifiers().testFlag(Qt::ControlModifier))
            _objectSet->showAllSelectedPatches(false);
        else
            _objectSet->showSelected(false);
        return true;

    case Qt::Key_S:
        if (e->modifiers().testFlag(Qt::ControlModifier) && e->modifiers().testFlag(Qt::ShiftModifier))
            _objectSet->showAll();
        else if (e->modifiers().testFlag(Qt::ControlModifier))
            _objectSet->showAllSelectedPatches(true);
        else
            _objectSet->showSelected(true);
        return true;

    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt: _glWidget->keyPressEvent(e); return true;
    default: return false;
    }
}
