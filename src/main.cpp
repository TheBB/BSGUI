#include <thread>
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

    for (int i = 1; i < argc; i++)
        window.objectSet()->loadFile(argv[i]);

    return app.exec();
}
