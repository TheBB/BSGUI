#include "dispobject.h"
#include "glwidget.h"


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


GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent),
                                      vcProgram(), ccProgram(),
                                      obj(),
                                      timer(this)
{
}


void GLWidget::paintGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vcProgram.bind();

    float el = elapsedTimer.elapsed() / 1000.0;
    float aspect = (float) width() / height();
    QMatrix4x4 mx;
    mx.perspective(45.0, aspect, 0.1, 100.0);
    mx.lookAt(QVector3D(0, -4, 2), QVector3D(0, 0, 0), QVector3D(0, 0, 1));
    mx.rotate(360.0 * el / 10, QVector3D(0, 0, 1));
    vcProgram.setUniformValue("mvp", mx);

    obj.draw(elapsedTimer.elapsed(), vcProgram);

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

    obj.init();

    elapsedTimer.start();

    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer.start(25);
}
