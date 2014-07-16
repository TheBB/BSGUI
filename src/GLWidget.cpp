#include <cmath>

#include "DispObject.h"
#include "shaders.h"

#include "GLWidget.h"


GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
    , vcProgram(), ccProgram(), lnProgram()
    , selectedObject(NULL)
    , shiftPressed(false)
    , ctrlPressed(false)
    , altPressed(false)
    , _zoom(0.0)
    , worldTrans(0,0,0)
    , _fov(45.0)
    , _inclination(30.0)
    , _azimuth(45.0)
    , cameraTracking(false)
{
    setFocusPolicy(Qt::ClickFocus);
}


GLWidget::~GLWidget()
{
    for (auto obj : objects)
        free(obj);
}


QSize GLWidget::sizeHint() const
{
    return QSize(640, 480);
}


void GLWidget::paintGL()
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 proj, mv;
    matrices(&proj, &mv);

    for (auto obj : objects)
        obj->draw(proj, mv, vcProgram, ccProgram, lnProgram);

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
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!vcProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vsVaryingColor))
        close();
    if (!vcProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fsVaryingColor))
        close();
    if (!vcProgram.link())
        close();

    if (!ccProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vsConstantColor))
        close();
    if (!ccProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fsConstantColor))
        close();
    if (!ccProgram.link())
        close();

    if (!lnProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, vsLines))
        close();
    if (!lnProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, fsLines))
        close();
    if (!lnProgram.link())
        close();

    std::vector<QVector3D> centers = {
        QVector3D(0, 0, 0),
        QVector3D(-3, 0, 0),
        QVector3D(0, -3, 0),
        QVector3D(6, 0, 0),
        QVector3D(0, 4, 0),
    };

    for (QVector3D c : centers)
    {
        DispObject *obj = new DispObject();
        obj->init(c);
        objects.insert(obj);
    }
}


void GLWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control)
        ctrlPressed = true;
    if (event->key() == Qt::Key_Shift)
        shiftPressed = true;
    if (event->key() == Qt::Key_Alt)
        altPressed = true;
}


void GLWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control)
        ctrlPressed = false;
    if (event->key() == Qt::Key_Shift)
        shiftPressed = false;
    if (event->key() == Qt::Key_Alt)
        altPressed = false;
}


void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() & Qt::RightButton)
    {
        cameraTracking = true;
        cameraOrig = event->pos();
        azimuthOrig = _azimuth;
        inclinationOrig = _inclination;
    }
    else if (event->button() & Qt::LeftButton)
    {
        QVector4D origin = QVector4D((float) event->pos().x() / width() * 2.0 - 1.0,
                                     - (float) event->pos().y() / height() * 2.0 + 1.0,
                                     0, 1);
        QVector4D point = origin; point.setZ(1);

        QMatrix4x4 proj, mv;
        matrices(&proj, &mv);

        bool inverted;
        QMatrix4x4 inv = (proj * mv).inverted(&inverted);

        if (!inverted)
            qDebug() << "Couldn't invert matrix";

        QVector3D a = (inv * origin).toVector3DAffine();
        QVector3D b = (inv * point).toVector3DAffine();

        DispObject *selected = NULL;
        float param, minParam = std::numeric_limits<float>::infinity();
        bool intersect;
        for (auto obj : objects)
        {
            obj->intersect(a, b, &intersect, &param);
            if (intersect && param < minParam && param >= 0.0)
            {
                selected = obj;
                minParam = param;
            }
        }

        bool needsUpdate = selectedObject || selected;

        if (selectedObject)
        {
            selectedObject->selected = false;
            selectedObject = NULL;
        }

        if (selected)
        {
            selected->selected = true;
            selectedObject = selected;
        }

        if (needsUpdate)
            update();
    }
}


void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() & Qt::RightButton)
        cameraTracking = false;
}


void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (cameraTracking)
    {
        setAzimuth(azimuthOrig + 360.0 * (event->pos().x() - cameraOrig.x()) / width(), true);
        setInclination(inclinationOrig + 180.0 * (event->pos().y() - cameraOrig.y()) / height(), true);
        update();
    }
}


void GLWidget::wheelEvent(QWheelEvent *event)
{
    if (ctrlPressed)
        setFov(fov() / exp((float) event->angleDelta().y() / 120.0 / (shiftPressed ? 150.0 : 15.0)), true);
    else
        setZoom(zoom() + (double) event->angleDelta().y() / 120.0 / (shiftPressed ? 400.0 : 40.0), true);

    update();
}


void GLWidget::setInclination(double val, bool fromMouse)
{
    if (val >= 90.0)
        val = 90.0;
    if (val <= -90.0)
        val = -90.0;
    _inclination = val;

    emit inclinationChanged(val, fromMouse);
}


void GLWidget::setAzimuth(double val, bool fromMouse)
{
    while (val > 360.0)
        val -= 360.0;
    while (val < 0.0)
        val += 360.0;
    _azimuth = val;

    emit azimuthChanged(val, fromMouse);
}


void GLWidget::setFov(double val, bool fromMouse)
{
    if (val >= MAX_FOV)
        val = MAX_FOV;
    if (val < 0.0)
        val = 0.0;
    _fov = val;

    emit fovChanged(val, fromMouse);
}


void GLWidget::setZoom(double val, bool fromMouse)
{
    _zoom = val;

    emit zoomChanged(val, fromMouse);
}


void GLWidget::matrices(QMatrix4x4 *proj, QMatrix4x4 *mv)
{
    float aspect = (float) width() / height();
    proj->setToIdentity();
    proj->perspective(_fov, aspect, 0.01, 100.0);

    QVector3D eye = QVector3D(0, _zoom, 0);
    mv->setToIdentity();
    mv->lookAt(eye, eye + QVector3D(0, 1, 0), QVector3D(0, 0, 1));
    mv->translate(QVector3D(0, 1, 0));
    mv->rotate(_inclination, QVector3D(1, 0, 0));
    mv->rotate(_azimuth, QVector3D(0, 0, 1));
    mv->scale(1.0/11.0);
    mv->translate(-worldTrans);
}
