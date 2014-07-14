#include <iostream>
#include <sstream>

#include <QtDebug>

#include <QApplication>
#include <QMainWindow>

#include <QGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include "main.h"


QString vertexShaderSource =
    "#version 130\n"
    "\n"
    "in vec2 vertexPosition;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = vec4(vertexPosition, 0.0, 1.0);\n"
    "}\n";

QString fragmentShaderSource =
    "#version 130\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "     gl_FragColor = vec4(0.0, 0.0, 1.0, 0.0);\n"
    "}\n";


class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = NULL);
    virtual ~GLWidget() { }

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    QOpenGLShaderProgram program;
    QOpenGLBuffer vertexBuffer;
};


GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent),
                                      program(),
                                      vertexBuffer(QOpenGLBuffer::VertexBuffer)
{
}


void GLWidget::paintGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    vertexBuffer.bind();
    program.enableAttributeArray("vertexPosition");
    program.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2);

    program.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
}


void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}


void GLWidget::initializeGL()
{
    if (!program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource))
        close();

    if (!program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource))
        close();

    if (!program.link())
        close();

    program.bind();

    float vertexData[] = {
        0.0, 0.8,
        -0.8, -0.8,
        0.8, -0.8
    };

    vertexBuffer.create();
    vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexBuffer.bind();
    vertexBuffer.allocate(vertexData, 3 * 2 * sizeof(float));
}


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
