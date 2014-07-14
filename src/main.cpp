#include <iostream>
#include <sstream>

#include <QtDebug>

#include <QApplication>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QTimer>

#include <QGLFormat>
#include <QGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include "main.h"


QString vertexShaderSource =
    "#version 130\n"
    "\n"
    "in vec2 vertexPosition;\n"
    "in vec3 vertexColor;\n"
    "\n"
    "out vec3 outColor;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    outColor = vertexColor;\n"
    "    gl_Position = vec4(vertexPosition, 0.0, 1.0);\n"
    "}\n";

QString fragmentShaderSource =
    "#version 130\n"
    "\n"
    "in vec3 outColor;\n"
    "uniform float fade;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor = vec4(outColor, fade);\n"
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
    QOpenGLBuffer colorBuffer;

    QTimer timer;
    QElapsedTimer elapsedTimer;
};


GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent),
                                      program(),
                                      vertexBuffer(QOpenGLBuffer::VertexBuffer),
                                      colorBuffer(QOpenGLBuffer::VertexBuffer),
                                      timer(this)
{
}


void GLWidget::paintGL()
{
    qDebug() << "paintGL " << elapsedTimer.elapsed();

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vertexBuffer.bind();
    program.enableAttributeArray("vertexPosition");
    program.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2);

    colorBuffer.bind();
    program.enableAttributeArray("vertexColor");
    program.setAttributeBuffer("vertexColor", GL_FLOAT, 0, 3);

    GLfloat elapsed = (elapsedTimer.elapsed() % 2000) / 1000.0;
    GLfloat fade = elapsed < 1.0 ? elapsed : 2.0 - elapsed;
    program.setUniformValue("fade", fade);

    program.bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    swapBuffers();
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
        0.0f, 0.8f,
        -0.8f, -0.8f,
        0.8f, -0.8f
    };

    vertexBuffer.create();
    vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexBuffer.bind();
    vertexBuffer.allocate(vertexData, 3 * 2 * sizeof(float));

    float colorData[] = {
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f
    };

    colorBuffer.create();
    colorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    colorBuffer.bind();
    colorBuffer.allocate(colorData, 3 * 3 * sizeof(float));

    elapsedTimer.start();

    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer.start(30);
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
