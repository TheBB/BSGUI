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

#include <sstream>
#include <thread>
#include <QApplication>
#include <QFileDialog>
#include <QMenuBar>
#include <QSettings>
#include <QSplitter>
#include <QTabWidget>
#include <QVector3D>

#include "main.h"

#include "MainWindow.h"


MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    qRegisterMetaType<LogLevel>("LogLevel");

    std::ostringstream ss;
    ss << "SINTEF BSGUI "
       << BSGUI_VERSION_MAJOR << "."
       << BSGUI_VERSION_MINOR << "."
       << BSGUI_VERSION_PATCH;
    setWindowTitle(QString(ss.str().c_str()));

    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    _settings = new QSettings("SINTEF", "BSGUI");

    _objectSet = new ObjectSet();

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    setCentralWidget(splitter);

    _glWidget = new GLWidget(_objectSet, this, _settings);
    splitter->addWidget(_glWidget);
    _glWidget->setFocus();

    _infoBox = new InfoBox(_objectSet, this);
    splitter->addWidget(_infoBox);

    splitter->setStretchFactor(0, 1.0);
    splitter->setStretchFactor(1, 0.0);

    _toolBox = new ToolBox(_glWidget, _objectSet, "Toolbox", this);
    _toolBox->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, _toolBox);


    QMenu *fileMenu = menuBar()->addMenu("File");
    QAction *openAct = fileMenu->addAction("Open");
    openAct->setShortcut(QKeySequence("Ctrl+O"));

    connect(openAct, &QAction::triggered,
            [this] (bool checked) {
                QStringList list = QFileDialog::getOpenFileNames(
                    this, "Open mesh files", ".", "GoTools files (*.g2);;All files (*)");
                for (auto f : list)
                    _objectSet->loadFile(f);
            });

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction("Exit");
    exitAct->setShortcut(QKeySequence("Ctrl+Q"));

    connect(exitAct, &QAction::triggered,
            [] () { QApplication::exit(0); });


    QMenu *windowsMenu = menuBar()->addMenu("Windows");
    _toolAct = windowsMenu->addAction("Toolbox");
    _toolAct->setShortcut(QKeySequence("Ctrl+Shift+T"));
    _toolAct->setCheckable(true);
    _infoAct = windowsMenu->addAction("Infobox");
    _infoAct->setShortcut(QKeySequence("Ctrl+Shift+I"));
    _infoAct->setCheckable(true);

    connect(_toolAct, &QAction::triggered,
            [this] (bool checked) { _toolBox->setVisible(checked); });
    connect(_infoAct, &QAction::triggered,
            [this] (bool checked) { _infoBox->setVisible(checked); });

    windowsMenu->addSeparator();

    _toggleAct = windowsMenu->addAction("Toggle full view");
    _toggleAct->setShortcut(QKeySequence("Ctrl+Shift+F"));
    _toggleAct->setCheckable(true);

    connect(_toggleAct, &QAction::triggered,
            [this] (bool checked) {
                _toolBox->setVisible(!checked);
                _infoBox->setVisible(!checked);
            });
}


MainWindow::~MainWindow()
{
    delete _objectSet;
    delete _glWidget;
    delete _settings;
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Show || event->type() == QEvent::Hide)
    {
        if (obj == _toolBox || obj == _infoBox)
        {
            if (obj == _toolBox)
                _toolAct->setChecked(event->type() == QEvent::Show);
            if (obj == _infoBox)
                _infoAct->setChecked(event->type() == QEvent::Show);
            _toggleAct->setChecked(!_infoBox->isVisible() && !_toolBox->isVisible());
        }

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

    case Qt::Key_Escape:
    {
        std::set<std::pair<uint,uint>> p;
        _objectSet->setSelection(&p, true);
        return true;
    }

    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt: _glWidget->keyPressEvent(e); return true;
    default: return false;
    }
}
