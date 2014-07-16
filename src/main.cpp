#include <QApplication>
#include <QGLFormat>

#include "MainWindow.h"


int main(int argc, char **argv)
{
    QGLFormat fmt;
    fmt.setRgba(true);
    fmt.setAlpha(true);
    fmt.setDepth(true);
    fmt.setDoubleBuffer(true);
    QGLFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);

    MainWindow window;
    window.showMaximized();

    return app.exec();
}
