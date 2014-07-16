#include <cmath>
#include <iostream>
#include <sstream>

#include <QtDebug>

#include <QApplication>
#include <QMainWindow>

#include <QGLFormat>

#include "GLWidget.h"
#include "main.h"


int main(int argc, char **argv)
{
    QGLFormat fmt;
    fmt.setRgba(true);
    fmt.setAlpha(true);
    fmt.setDepth(true);
    fmt.setDoubleBuffer(true);
    QGLFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);

    QMainWindow window;

    std::ostringstream ss;
    ss << "SINTEF BSGUI "
       << BSGUI_VERSION_MAJOR << "."
       << BSGUI_VERSION_MINOR << "."
       << BSGUI_VERSION_PATCH;
    window.setWindowTitle(QString(ss.str().c_str()));

    GLWidget glWidget;
    window.setCentralWidget((QWidget *) &glWidget);
    glWidget.setFocus();

    window.show();

    return app.exec();
}
