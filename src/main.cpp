#include <thread>
#include <QApplication>
#include <QGLFormat>

#include "MainWindow.h"


void loadFiles(MainWindow *window, int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
        window->objectSet()->addPatchesFromFile(argv[i]);
    window->glWidget()->centerOnSelected();
}


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

    new std::thread(loadFiles, &window, argc, argv);

    return app.exec();
}
