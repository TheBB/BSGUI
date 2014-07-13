#include <iostream>
#include <sstream>

#include <QApplication>
#include <QMainWindow>
#include <QGLWidget>

#include "main.h"


class GLWidget : public QGLWidget
{
    Q_OBJECT
    
public:
    GLWidget(QWidget *parent = NULL) : QGLWidget(parent) { }
    virtual ~GLWidget() { }
    
protected:
    void initializeGL()
    {
        std::cout << "initializeGL" << std::endl;
    }
    
    void resizeGL(int w, int h)
    {
        std::cout << "resizeGL" << std::endl;
    }
    
    void paintGL()
    {
        std::cout << "paintGL" << std::endl;
    }
};


int main(int argc, char **argv)
{
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
    
    window.show();

    return app.exec();
}


#include "main.moc"
