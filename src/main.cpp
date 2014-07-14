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
    "in vec2 vertexPosition;\n"
    "in vec3 vertexColor;\n"
    "uniform mat4 mvp;\n"
    "\n"
    "out vec3 outColor;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    outColor = vertexColor;\n"
    "    gl_Position = mvp * vec4(vertexPosition, 0.0, 1.0);\n"
    "}\n";

QString fragmentShaderSourceVarCol =
    "#version 130\n"
    "\n"
    "in vec3 outColor;\n"
    "uniform float fade;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor = vec4(outColor, fade);\n"
    "}\n";

QString vertexShaderSourceConstCol =
    "#version 130\n"
    "\n"
    "in vec2 vertexPosition;\n"
    "uniform mat4 mvp;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = mvp * vec4(vertexPosition, 0.0, 1.0);\n"
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

    QTimer timer;
    QElapsedTimer elapsedTimer;
};


GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent),
                                      vcProgram(), ccProgram(),
                                      vertexBuffer(QOpenGLBuffer::VertexBuffer),
                                      colorBuffer(QOpenGLBuffer::VertexBuffer),
                                      timer(this)
{
}


void GLWidget::paintGL()
{
    // qDebug() << "paintGL " << elapsedTimer.elapsed();

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat fade = (elapsedTimer.elapsed() % 2000) / 1000.0;
    fade = fade < 1.0 ? fade : 2.0 - fade;

    float el = elapsedTimer.elapsed() / 1000.0;
    QMatrix4x4 mx;
    mx.translate(sin(2 * 3.14159265 * el / 7), 0, 0);
    mx.rotate(360.0 / 10 * el, 0, 0, 1);

    vcProgram.bind();

    vertexBuffer.bind();
    vcProgram.enableAttributeArray("vertexPosition");
    vcProgram.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2);

    colorBuffer.bind();
    vcProgram.enableAttributeArray("vertexColor");
    vcProgram.setAttributeBuffer("vertexColor", GL_FLOAT, 0, 3);

    vcProgram.setUniformValue("mvp", mx);
    vcProgram.setUniformValue("fade", fade);

    glDrawArrays(GL_TRIANGLES, 0, 3);


    // ccProgram.bind();

    // vertexBuffer.bind();
    // ccProgram.enableAttributeArray("vertexPosition");
    // ccProgram.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2);

    // ccProgram.setUniformValue("col", QVector4D(0.0, 0.0, 1.0, 1.0 - fade));

    // glDrawArrays(GL_TRIANGLES, 0, 3);


    swapBuffers();
}


void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}


void GLWidget::initializeGL()
{
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

    vcProgram.bind();

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
