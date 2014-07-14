#include <cmath>
#include <iostream>
#include <sstream>

#include <QtDebug>

#include <QApplication>
#include <QElapsedTimer>
#include <QMainWindow>
#include <QTimer>
#include <QVector4D>

#include <QGLFormat>
#include <QGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#include "main.h"


QString vertexShaderSourceVarCol =
    "#version 130\n"
    "\n"
    "in vec3 vertexPosition;\n"
    "in vec3 vertexColor;\n"
    "uniform mat4 mvp;\n"
    "\n"
    "out vec3 outColor;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    outColor = vertexColor;\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

QString fragmentShaderSourceVarCol =
    "#version 130\n"
    "\n"
    "in vec3 outColor;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor = vec4(outColor, 1.0);\n"
    "}\n";

QString vertexShaderSourceConstCol =
    "#version 130\n"
    "\n"
    "in vec3 vertexPosition;\n"
    "uniform mat4 mvp;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

QString fragmentShaderSourceConstCol =
    "#version 130\n"
    "\n"
    "uniform vec4 col;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor = col;\n"
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
    QOpenGLShaderProgram vcProgram;
    QOpenGLShaderProgram ccProgram;
    QOpenGLBuffer vertexBuffer;
    QOpenGLBuffer colorBuffer;
    QOpenGLBuffer indexBuffer;

    QTimer timer;
    QElapsedTimer elapsedTimer;
};


GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent),
                                      vcProgram(), ccProgram(),
                                      vertexBuffer(QOpenGLBuffer::VertexBuffer),
                                      colorBuffer(QOpenGLBuffer::VertexBuffer),
                                      indexBuffer(QOpenGLBuffer::IndexBuffer),
                                      timer(this)
{
}


void GLWidget::paintGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vcProgram.bind();

    vertexBuffer.bind();
    vcProgram.enableAttributeArray("vertexPosition");
    vcProgram.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    colorBuffer.bind();
    vcProgram.enableAttributeArray("vertexColor");
    vcProgram.setAttributeBuffer("vertexColor", GL_FLOAT, 0, 3);

    indexBuffer.bind();

    float el = elapsedTimer.elapsed() / 1000.0;
    float aspect = (float) width() / height();
    QMatrix4x4 mx;
    mx.perspective(45.0, aspect, 0.1, 100.0);
    mx.lookAt(QVector3D(0, -4, 2), QVector3D(0, 0, 0), QVector3D(0, 0, 1));
    mx.rotate(360.0 * el / 10, QVector3D(0, 0, 1));
    vcProgram.setUniformValue("mvp", mx);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);

    swapBuffers();
}


void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}


void GLWidget::initializeGL()
{
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!vcProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSourceVarCol))
        close();
    if (!vcProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSourceVarCol))
        close();
    if (!vcProgram.link())
        close();

    if (!ccProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSourceConstCol))
        close();
    if (!ccProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSourceConstCol))
        close();
    if (!ccProgram.link())
        close();

    float vertexData[] = {
        -1.0, -1.0,  1.0,
        1.0, -1.0,  1.0,
        1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,
        -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0,  1.0, -1.0,
        -1.0,  1.0, -1.0,
    };

    vertexBuffer.create();
    vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexBuffer.bind();
    vertexBuffer.allocate(vertexData, 8 * 3 * sizeof(float));

    float colorData[] = {
        0.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        0.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
    };

    colorBuffer.create();
    colorBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    colorBuffer.bind();
    colorBuffer.allocate(colorData, 8 * 3 * sizeof(float));

    GLushort indexData[] = {
        0, 1, 2,
        2, 3, 0,
        3, 2, 6,
        6, 7, 3,
        7, 6, 5,
        5, 4, 7,
        4, 5, 1,
        1, 0, 4,
        4, 0, 3,
        3, 7, 4,
        1, 5, 6,
        6, 2, 1
    };

    indexBuffer.create();
    indexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    indexBuffer.bind();
    indexBuffer.allocate(indexData, 12 * 3 * sizeof(GLushort));

    elapsedTimer.start();

    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer.start(25);
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
