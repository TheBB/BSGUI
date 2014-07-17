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
    , _inclination(30.0)
    , _azimuth(45.0)
    , _fov(45.0)
    , _roll(0.0)
    , _zoom(0.0)
    , _lookAt(1.5,0,0)
    , _perspective(true)
    , _fixed(false)
    , _dir(POSZ)
    , _rightHanded(true)
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


void GLWidget::centerOnSelected()
{
    if (selectedObject)
    {
        _lookAt = selectedObject->center();
        emit lookAtChanged(_lookAt, true);
    }
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
        QVector3D(0, 0, 1),
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
    if (event->key() == Qt::Key_C)
    {
        centerOnSelected();
        update();
    }
    if (event->key() == Qt::Key_QuoteLeft)
        usePreset(VIEW_FREE);
    if (event->key() == Qt::Key_1)
        usePreset(VIEW_TOP);
    if (event->key() == Qt::Key_2)
        usePreset(VIEW_BOTTOM);
    if (event->key() == Qt::Key_3)
        usePreset(VIEW_LEFT);
    if (event->key() == Qt::Key_4)
        usePreset(VIEW_RIGHT);
    if (event->key() == Qt::Key_5)
        usePreset(VIEW_FRONT);
    if (event->key() == Qt::Key_6)
        usePreset(VIEW_BACK);
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
        mouseOrig = event->pos();
        mouseOrigAzimuth = _azimuth;
        mouseOrigInclination = _inclination;
        mouseOrigRoll = _roll;
        mouseOrigLookAt = _lookAt;
    }
    else if (event->button() & Qt::LeftButton)
    {
        QVector4D origin = QVector4D((float) event->pos().x() / width() * 2.0 - 1.0,
                                     - (float) event->pos().y() / height() * 2.0 + 1.0,
                                     0, 1);
        QVector4D point = origin; point.setZ(1);

        QMatrix4x4 proj, mv, inv;
        matrices(&proj, &mv);
        inv = (proj * mv).inverted();

        QVector3D a = (inv * origin).toVector3DAffine();
        QVector3D b = (inv * point).toVector3DAffine();

        if (!_perspective)
        {
            QVector3D diff = 2*(a - b);
            a += diff;
            b += diff;
        }

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

        emit singlePatchSelected(selected);
    }
}


void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() & Qt::RightButton)
        cameraTracking = false;
}


void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!cameraTracking)
        return;

    if (ctrlPressed && !_fixed || !ctrlPressed && _fixed)
    {
        QMatrix4x4 proj, mv, inv;
        matrices(&proj, &mv);
        inv = (proj * mv).inverted();

        QVector3D right = -(inv * QVector4D(1,0,0,0)).toVector3D(); right.normalize();
        QVector3D up = (inv * QVector4D(0,1,0,0)).toVector3D(); up.normalize();

        setLookAt(mouseOrigLookAt + (shiftPressed ? 1.0 : 10.0) * (
                      right * (event->pos().x() - mouseOrig.x()) / width() +
                      up    * (event->pos().y() - mouseOrig.y()) / height()),
                  true);
    }
    else if (!_fixed)
    {
        setAzimuth(mouseOrigAzimuth + (shiftPressed ? 36.0 : 360.0) *
                   (event->pos().x() - mouseOrig.x()) / width(), true);
        setInclination(mouseOrigInclination + (shiftPressed ? 18.0 : 180.0) *
                       (event->pos().y() - mouseOrig.y()) / height(), true);
    }
    else
        setRoll(mouseOrigRoll + (shiftPressed ? 36.0 : 360.0) *
                (event->pos().x() - mouseOrig.x()) / width(), true);

    update();
}


void GLWidget::wheelEvent(QWheelEvent *event)
{
    if (ctrlPressed || !_perspective)
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


void GLWidget::setRoll(double val, bool fromMouse)
{
    while (val > 360.0)
        val -= 360.0;
    while (val < 0.0)
        val += 360.0;
    _roll = val;

    emit rollChanged(val, fromMouse);
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


void GLWidget::setLookAt(QVector3D pt, bool fromMouse)
{
    _lookAt = pt;
    emit lookAtChanged(pt, fromMouse);
}


void GLWidget::setPerspective(bool val)
{
    _perspective = val;

    if (val)
    {
        _zoom = 1.0 - (1.0 - _zoom) * tan(_fov * 3.14159265 / 360.0) / tan(orthoOrigFov * 3.14159265 / 360.0);
        _fov = orthoOrigFov;
        emit zoomChanged(_zoom, true);
        emit fovChanged(_fov, true);
    }
    else
        orthoOrigFov = _fov;

    emit perspectiveChanged(val);
}


void GLWidget::usePreset(preset val)
{
    if (val == VIEW_FREE)
    {
        if (!_fixed)
            return;

        setInclination(fixedOrigInclination, true);
        setAzimuth(fixedOrigAzimuth, true);
        setRoll(fixedOrigRoll, true);
        setFov(fixedOrigFov, true);
        setZoom(fixedOrigZoom, true);

        setPerspective(fixedOrigPerspective);

        _fixed = false;
        emit fixedChanged(_fixed, val);

        return;
    }
    
    if (!_fixed)
    {
        fixedOrigInclination = _inclination;
        fixedOrigAzimuth = _azimuth;
        fixedOrigRoll = _roll;
        fixedOrigFov = _fov;
        fixedOrigZoom = _zoom;
        fixedOrigPerspective = _perspective;
    }

    setPerspective(false);
    setRoll(0.0, true);

    switch (val)
    {
    case VIEW_TOP:
        setInclination(90.0, true); break;
    case VIEW_BOTTOM:
        setInclination(-90.0, true); break;
    default:
        setInclination(0.0, true);
    }

    switch (val)
    {
    case VIEW_TOP:
    case VIEW_BOTTOM:
        setAzimuth(0.0, true); break;
    case VIEW_LEFT:
        setAzimuth(0.0, true); break;
    case VIEW_RIGHT:
        setAzimuth(180.0, true); break;
    case VIEW_FRONT:
        setAzimuth(90.0, true); break;
    case VIEW_BACK:
        setAzimuth(270.0, true);
    }

    _fixed = true;
    emit fixedChanged(_fixed, val);

    update();
}


void GLWidget::setDir(direction val)
{
    _dir = val;
    emit dirChanged(val);
}


void GLWidget::setRightHanded(bool val)
{
    _rightHanded = val;
    emit rightHandedChanged(val);
}


void GLWidget::matrices(QMatrix4x4 *proj, QMatrix4x4 *mv)
{
    float aspect = (float) width() / height();
    proj->setToIdentity();
    if (_perspective)
        proj->perspective(_fov, aspect, 0.01, 100.0);
    else
    {
        float h = (1.0 - _zoom) * tan(_fov * 3.14159265 / 360.0);
        proj->ortho(-aspect * h, aspect * h, -h, h, 0.01, 100.0);
    }

    QVector3D eye = QVector3D(0, (_perspective ? _zoom : 0.0), 0);
    mv->setToIdentity();
    mv->lookAt(eye, eye + QVector3D(0, 1, 0), QVector3D(0, 0, 1));
    mv->translate(QVector3D(0, 1, 0));
    mv->rotate(_roll, QVector3D(0, 1, 0));
    mv->rotate(_inclination, QVector3D(1, 0, 0));
    mv->rotate(_azimuth, QVector3D(0, 0, 1));
    mv->scale(1.0/11.0);
    multiplyDir(mv);
    mv->translate(-_lookAt);
}


void GLWidget::multiplyDir(QMatrix4x4 *mv)
{
    if (_rightHanded)
        switch (_dir)
        {
        case POSX: (*mv) *= QMatrix4x4(0,0,-1,0,0,1,0,0,1,0,0,0,0,0,0,1); break;
        case NEGX: (*mv) *= QMatrix4x4(0,0,1,0,0,1,0,0,-1,0,0,0,0,0,0,1); break;
        case POSY: (*mv) *= QMatrix4x4(1,0,0,0,0,0,-1,0,0,1,0,0,0,0,0,1); break;
        case NEGY: (*mv) *= QMatrix4x4(1,0,0,0,0,0,1,0,0,-1,0,0,0,0,0,1); break;
        case NEGZ: (*mv) *= QMatrix4x4(-1,0,0,0,0,1,0,0,0,0,-1,0,0,0,0,1);
        }
    else
        switch (_dir)
        {
        case POSX: (*mv) *= QMatrix4x4(0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,1); break;
        case NEGX: (*mv) *= QMatrix4x4(0,0,-1,0,0,1,0,0,-1,0,0,0,0,0,0,1); break;
        case POSY: (*mv) *= QMatrix4x4(1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1); break;
        case NEGY: (*mv) *= QMatrix4x4(1,0,0,0,0,0,-1,0,0,-1,0,0,0,0,0,1); break;
        case POSZ: (*mv) *= QMatrix4x4(-1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1); break;
        case NEGZ: (*mv) *= QMatrix4x4(1,0,0,0,0,1,0,0,0,0,-1,0,0,0,0,1);
        }
}
