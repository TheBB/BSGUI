/*
 * Copyright (C) 2014 SINTEF ICT,
 * Applied Mathematics, Norway.
 *
 * Contact information:
 * E-mail: eivind.fonn@sintef.no
 * SINTEF ICT, Department of Applied Mathematics,
 * P.O. Box 4760 Sluppen,
 * 7045 Trondheim, Norway.
 *
 * This file is part of BSGUI.
 *
 * BSGUI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * BSGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with GoTools. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In accordance with Section 7(b) of the GNU Affero General Public
 * License, a covered work must retain the producer line in every data
 * file that is created or manipulated using GoTools.
 *
 * Other Usage
 * You can be released from the requirements of the license by purchasing
 * a commercial license. Buying such a license is mandatory as soon as you
 * develop commercial activities involving the GoTools library without
 * disclosing the source code of your own applications.
 *
 * This file may be used in accordance with the terms contained in a
 * written agreement between you and SINTEF ICT.
 */

#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <QFile>
#include <QTextStream>
#include <QRect>
#include <QDesktopWidget>

#include "DisplayObject.h"
#include "GLutils.h"

#include "GLWidget.h"

#define AW 0.025

GLWidget::GLWidget(QGLFormat fmt, ObjectSet *oSet, QWidget *parent)
    : QGLWidget(fmt, parent)
    , vcProgram(), ccProgram()
    , auxBuffer(QOpenGLBuffer::VertexBuffer)
    , axesBuffer(QOpenGLBuffer::IndexBuffer)
    , selectionBuffer(QOpenGLBuffer::IndexBuffer)
    , auxCBuffer(QOpenGLBuffer::VertexBuffer)
    , vao()
    , objectSet(oSet)
    , shiftPressed(false)
    , ctrlPressed(false)
    , altPressed(false)
    , _inclination(30.0)
    , _azimuth(45.0)
    , _fov(45.0)
    , _roll(0.0)
    , _zoom(1.0)
    , _lookAt(0,0,0)
    , _perspective(true)
    , _fixed(false)
    , _dir(POSZ)
    , _rightHanded(true)
    , _showAxes(true)
    , _showPoints(false)
    , _diameter(20.0)
    , selectTracking(false)
    , cameraTracking(false)
{
    installEventFilter(parent);
    setFocusPolicy(Qt::ClickFocus);
    QObject::connect(oSet, &ObjectSet::requestInitialization, this, &GLWidget::initializeDispObject);
    QObject::connect(oSet, SIGNAL(update()), this, SLOT(update()));
    QObject::connect(oSet, SIGNAL(selectionChanged()), this, SLOT(update()));
}


GLWidget::~GLWidget()
{
    vao.destroy();
}


QSize GLWidget::sizeHint() const
{
    return QSize(640, 480);
}


void GLWidget::centerOnSelected()
{
    QVector3D center;
    float radius;
    objectSet->boundingSphere(&center, &radius);

    _lookAt = center;
    emit lookAtChanged(_lookAt, true);

    if (!objectSet->hasSelection())
    {
        if (radius > 0.0)
            _diameter = 2.0 * radius;
        else
            _diameter = 1.0;

        setFov(45.0);
        setZoom(1.0);
    }

    update();
}


std::set<std::pair<uint,uint>> GLWidget::paintGLPicks(int x, int y, int w, int h)
{
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_MULTISAMPLE);

    QMatrix4x4 mvp;
    matrix(&mvp);

    for (auto i = DisplayObject::begin(); i != DisplayObject::end(); i++)
        i->second->drawPicking(mvp, ccProgram, objectSet->selectionMode());

    GLubyte pixels[4 * w * h];
    glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    std::unordered_map<uint, uint> picks;
    for (int i = 0; i < w * h; i++)
    {
        uint key = DisplayObject::colorToKey(&pixels[4*i]);

        if (picks.find(key) != picks.end())
            picks[key]++;
        else
            picks[key] = 1;
    }

    std::set<uint> deletes;

    int limit = std::min(std::min(w, h) - 1, 2);
    for (auto p : picks)
        if (p.second < limit || p.first == WHITE_KEY)
            deletes.insert(p.first);

    for (auto p : deletes)
        picks.erase(p);
    
    std::set<std::pair<uint,uint>> ret;
    for (auto p : picks)
    {
        uint index, offset;
        DisplayObject::keyToIndex(p.first, &index, &offset);
        ret.insert(std::pair<uint,uint>(index, offset));
    }

    glEnable(GL_MULTISAMPLE);

    return ret;
}


void GLWidget::paintGL()
{
    checkErrors("paintGL.init");

    std::lock(m, DisplayObject::m);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    checkErrors("paintGL.postClear");

    QMatrix4x4 mvp;
    matrix(&mvp);

    for (auto i = DisplayObject::begin(); i != DisplayObject::end(); i++)
        i->second->draw(mvp, ccProgram, _showPoints || objectSet->selectionMode() == SM_POINT);
    checkErrors("paintGL.postLoop");

    if (_showAxes)
    {
        glDisable(GL_DEPTH_TEST);
        drawAxes();
        glEnable(GL_DEPTH_TEST);
        checkErrors("paintGL.postAxes");
    }

    if (selectTracking)
    {
        glDisable(GL_DEPTH_TEST);
        drawSelection();
        glEnable(GL_DEPTH_TEST);
        checkErrors("paintGL.postSelection");
    }

    swapBuffers();

    m.unlock();
    DisplayObject::m.unlock();

    checkErrors("paintGL.fin");
}


void GLWidget::drawAxes()
{
    vcProgram.bind();

    vao.bind();

    auxBuffer.bind();
    vcProgram.enableAttributeArray("vertexPosition");
    vcProgram.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    auxCBuffer.bind();
    vcProgram.enableAttributeArray("vertexColor");
    vcProgram.setAttributeBuffer("vertexColor", GL_FLOAT, 0, 3);

    QMatrix4x4 mvp;
    axesMatrix(&mvp);
    vcProgram.setUniformValue("mvp", mvp);

    axesBuffer.bind();
    checkErrors("drawAxes.preLineWidth");

    glLineWidth((GLfloat) 1.0);
    checkErrors("drawAxes.postLineWidth");
    glDrawElements(GL_TRIANGLES, 36 * 3, GL_UNSIGNED_INT, 0);

    vao.release();
}


void GLWidget::drawSelection()
{
    ccProgram.bind();

    auxBuffer.bind();
    ccProgram.enableAttributeArray("vertexPosition");
    ccProgram.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    QMatrix4x4 mvp;
    mvp.setToIdentity();

    QPoint d = selectTo - selectOrig;
    mvp.translate((float) selectOrig.x()/width() * 2.0 - 1.0,
                  1.0 - (float) selectOrig.y()/height() * 2.0, 0.0);
    mvp.scale((float) d.x()/width()*2.0, - (float) d.y()/height()*2.0, 1.0);
    ccProgram.setUniformValue("mvp", mvp);

    ccProgram.setUniformValue("col", QVector3D(0,0,0));

    selectionBuffer.bind();
    glLineWidth(1.0);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0);
}


void GLWidget::resizeGL(int w, int h)
{
    checkErrors("resizeGL.init");

    m.lock();
    glViewport(0, 0, w, h);
    m.unlock();

    checkErrors("resizeGL.fin");
}


bool addShader(QOpenGLShaderProgram &program, QOpenGLShader::ShaderType type, QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    QString source = stream.readAll();
    return program.addShaderFromSourceCode(type, source);
}


void GLWidget::initializeGL()
{
    checkErrors("initializeGL.init");

    QGLFormat fmt = format();
    QGLFormat afmt = context()->format();

    std::cout << "OpenGL context is " << (context()->isValid() ? "valid" : "invalid") << std::endl;
    std::cout << "Asked for OpenGL v" << fmt.majorVersion() << "." << fmt.minorVersion()
              << " and got v" << afmt.majorVersion() << "." << afmt.minorVersion() << std::endl;
    std::cout << "          Vendor: " << (const char*)glGetString(GL_VENDOR) << std::endl;
    std::cout << "        Renderer: " << (const char*)glGetString(GL_RENDERER) << std::endl;
    std::cout << "         Version: " << (const char*)glGetString(GL_VERSION) << std::endl;
    std::cout << "    GLSL version: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    checkErrors("initializeGL.postCheck");

    m.lock();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_MULTISAMPLE);

    checkErrors("initializeGL.postEnable");

    if (!addShader(vcProgram, QOpenGLShader::Vertex, ":/shaders/varying_vertex.glsl") ||
        !addShader(vcProgram, QOpenGLShader::Fragment, ":/shaders/varying_fragment.glsl") ||
        !vcProgram.link())
    {
        close();
    }

    checkErrors("initializeGL.postVCLink");

    if (!addShader(ccProgram, QOpenGLShader::Vertex, ":/shaders/constant_vertex.glsl") ||
        !addShader(ccProgram, QOpenGLShader::Fragment, ":/shaders/constant_fragment.glsl") ||
        !ccProgram.link())
    {
        close();
    }

    checkErrors("initializeGL.postCCLink");

    initializeAux();

    m.unlock();

    checkErrors("initializeGL.fin");
}


void GLWidget::initializeAux()
{
    vao.create();
    vao.bind();

    std::vector<QVector3D> auxData = {
        // X-axis
        QVector3D(AW, -AW, -AW), QVector3D(AW, AW, -AW),
        QVector3D(AW, -AW, AW), QVector3D(AW, AW, AW),
        QVector3D(1.0, -AW, -AW), QVector3D(1.0, AW, -AW),
        QVector3D(1.0, -AW, AW), QVector3D(1.0, AW, AW),

        // Y-axis
        QVector3D(-AW, AW, -AW), QVector3D(AW, AW, -AW),
        QVector3D(-AW, AW, AW), QVector3D(AW, AW, AW),
        QVector3D(-AW, 1.0, -AW), QVector3D(AW, 1.0, -AW),
        QVector3D(-AW, 1.0, AW), QVector3D(AW, 1.0, AW),

        // Z-axis
        QVector3D(-AW, -AW, AW), QVector3D(AW, -AW, AW),
        QVector3D(-AW, AW, AW), QVector3D(AW, AW, AW),
        QVector3D(-AW, -AW, 1.0), QVector3D(AW, -AW, 1.0),
        QVector3D(-AW, AW, 1.0), QVector3D(AW, AW, 1.0),
    };
    auxBuffer.create();
    auxBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    auxBuffer.bind();
    auxBuffer.allocate(&auxData[0], 24 * 3 * sizeof(float));
    checkErrors("initializeAux.postAuxBuffer");

    std::vector<GLuint> axesData = {
        // X-axis
        0, 1, 2, 1, 3, 2, 4, 5, 6, 5, 7, 6,
        0, 1, 4, 1, 5, 4, 2, 3, 6, 3, 7, 6,
        1, 5, 3, 5, 7, 3, 0, 4, 2, 4, 6, 2,

        // Y-axis
        8, 9, 10, 9, 11, 10, 12, 13, 14, 13, 15, 14,
        8, 9, 12, 9, 13, 12, 10, 11, 14, 11, 15, 14,
        9, 13, 11, 13, 15, 11, 8, 12, 10, 12, 14, 10,

        // Z-axis
        16, 17, 18, 17, 19, 18, 20, 21, 22, 21, 23, 22,
        16, 17, 20, 17, 21, 20, 18, 19, 22, 19, 23, 22,
        17, 21, 19, 21, 23, 19, 16, 20, 18, 20, 22, 18,
    };
    axesBuffer.create();
    axesBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    axesBuffer.bind();
    axesBuffer.allocate(&axesData[0], 36 * 3 * sizeof(GLuint));
    checkErrors("initializeAux.postAxesBuffer");

    // std::vector<GLuint> selectionData = {0, 1, 6, 3};
    // selectionBuffer.create();
    // selectionBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    // selectionBuffer.bind();
    // selectionBuffer.allocate(&selectionData[0], 4 * sizeof(GLuint));
    // checkErrors("initializeGL.postSelectionBuffer");

    std::vector<QVector3D> auxColors = {
        // X-axis
        QVector3D(1,0,0), QVector3D(1,0,0),
        QVector3D(1,0,0), QVector3D(1,0,0),
        QVector3D(1,0,0), QVector3D(1,0,0),
        QVector3D(1,0,0), QVector3D(1,0,0),

        // Y-axis
        QVector3D(0,1,0), QVector3D(0,1,0),
        QVector3D(0,1,0), QVector3D(0,1,0),
        QVector3D(0,1,0), QVector3D(0,1,0),
        QVector3D(0,1,0), QVector3D(0,1,0),

        // Z-axis
        QVector3D(0,0,1), QVector3D(0,0,1),
        QVector3D(0,0,1), QVector3D(0,0,1),
        QVector3D(0,0,1), QVector3D(0,0,1),
        QVector3D(0,0,1), QVector3D(0,0,1),
    };
    auxCBuffer.create();
    auxCBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    auxCBuffer.bind();
    auxCBuffer.allocate(&auxColors[0], 24 * 3 * sizeof(float));

    vao.release();
}


void GLWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control)
    {
        ctrlPressed = true;
        return;
    }
    if (event->key() == Qt::Key_Shift)
    {
        shiftPressed = true;
        return;
    }
    if (event->key() == Qt::Key_Alt)
    {
        altPressed = true;
        return;
    }

    update();
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
    if (event->button() == Qt::RightButton)
    {
        cameraTracking = true;
        mouseOrig = event->pos();
        mouseOrigAzimuth = _azimuth;
        mouseOrigInclination = _inclination;
        mouseOrigRoll = _roll;
        mouseOrigLookAt = _lookAt;
    }
    else if (event->button() == Qt::LeftButton)
    {
        selectTracking = true;
        selectOrig = event->pos();
        selectTo = event->pos();
    }
}


void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        cameraTracking = false;
    else if (event->button() == Qt::LeftButton)
    {
        std::lock(m, DisplayObject::m);

        selectTracking = false;

        int x = std::max(std::min(event->pos().x(), selectOrig.x()), 0);
        int y = std::max(height() - std::max(event->pos().y(), selectOrig.y()), 0);
        int toX = std::min(std::max(event->pos().x(), selectOrig.x()), width() - 1);
        int toY = std::min(height() - std::min(event->pos().y(), selectOrig.y()), height() - 1);

        makeCurrent();
        std::set<std::pair<uint,uint>> picks = paintGLPicks(x, y, toX - x + 1, toY - y + 1);

        m.unlock();
        DisplayObject::m.unlock();

        objectSet->setSelection(&picks, !ctrlPressed);
    }
}


void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (selectTracking)
    {
        selectTo = event->pos();
        update();
    }

    if (!cameraTracking)
        return;

    QDesktopWidget widget;
    QRect screen = widget.availableGeometry();

    if (ctrlPressed && !_fixed || !ctrlPressed && _fixed)
    {
        QMatrix4x4 mvp, inv;
        matrix(&mvp);
        inv = mvp.inverted();

        QVector3D right = -(inv * QVector4D(1,0,0,0)).toVector3D(); right.normalize();
        QVector3D up = (inv * QVector4D(0,1,0,0)).toVector3D(); up.normalize();

        float fac = _zoom * tan(_fov * 3.14159265 / 360.0) * _diameter;

        setLookAt(mouseOrigLookAt + (shiftPressed ? 0.2 : 2.0) * fac / height() * (
                      right * (event->pos().x() - mouseOrig.x()) +
                      up    * (event->pos().y() - mouseOrig.y())),
                  true);
    }
    else if (!_fixed)
    {
        setAzimuth(mouseOrigAzimuth + (shiftPressed ? 36.0 : 360.0) *
                   (event->pos().x() - mouseOrig.x()) / screen.width());
        setInclination(mouseOrigInclination + (shiftPressed ? 18.0 : 180.0) *
                       (event->pos().y() - mouseOrig.y()) / screen.height());
    }
    else
        setRoll(mouseOrigRoll - (shiftPressed ? 36.0 : 360.0) *
                (event->pos().x() - mouseOrig.x()) / screen.width());

    update();
}


void GLWidget::wheelEvent(QWheelEvent *event)
{
    if (abs(event->angleDelta().y()) > 1000)
        return;

    if (ctrlPressed || !_perspective)
        setFov(fov() / exp((float) event->angleDelta().y() / 120.0 / (shiftPressed ? 150.0 : 15.0)));
    else
        setZoom(zoom() - (double) event->angleDelta().y() / 120.0 / (shiftPressed ? 400.0 : 40.0));

    update();
}


void GLWidget::setInclination(double val)
{
    if (val >= 90.0)
        val = 90.0;
    if (val <= -90.0)
        val = -90.0;
    _inclination = val;

    emit inclinationChanged(val);
    update();
}


void GLWidget::setAzimuth(double val)
{
    while (val > 360.0)
        val -= 360.0;
    while (val < 0.0)
        val += 360.0;
    _azimuth = val;

    emit azimuthChanged(val);
    update();
}


void GLWidget::setRoll(double val)
{
    while (val > 360.0)
        val -= 360.0;
    while (val < 0.0)
        val += 360.0;
    _roll = val;

    emit rollChanged(val);
    update();
}


void GLWidget::setFov(double val)
{
    if (val >= MAX_FOV)
        val = MAX_FOV;
    if (val < 0.0)
        val = 0.0;
    _fov = val;

    emit fovChanged(val);
    update();
}


void GLWidget::setZoom(double val)
{
    if (val > MAX_ZOOM)
        val = MAX_ZOOM;
    if (val < 0.0)
        val = 0.0;
    _zoom = val;

    emit zoomChanged(val);
    update();
}


void GLWidget::setLookAt(QVector3D pt, bool fromMouse)
{
    _lookAt = pt;

    emit lookAtChanged(pt, fromMouse);
    update();
}


void GLWidget::setPerspective(bool val)
{
    _perspective = val;

    if (val)
    {
        _zoom = _zoom * tan(_fov * 3.14159265 / 360.0) / tan(orthoOrigFov * 3.14159265 / 360.0);
        _fov = orthoOrigFov;
        emit zoomChanged(_zoom);
        emit fovChanged(_fov);
    }
    else
        orthoOrigFov = _fov;

    emit perspectiveChanged(val);
    update();
}


void GLWidget::usePreset(preset val)
{
    if (val == VIEW_FREE)
    {
        if (!_fixed)
            return;

        setInclination(fixedOrigInclination);
        setAzimuth(fixedOrigAzimuth);
        setRoll(fixedOrigRoll);
        setFov(fixedOrigFov);
        setZoom(fixedOrigZoom);

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
    setRoll(0.0);

    switch (val)
    {
    case VIEW_TOP:
        setInclination(90.0); break;
    case VIEW_BOTTOM:
        setInclination(-90.0); break;
    default:
        setInclination(0.0);
    }

    switch (val)
    {
    case VIEW_TOP:
    case VIEW_BOTTOM:
        setAzimuth(0.0); break;
    case VIEW_LEFT:
        setAzimuth(0.0); break;
    case VIEW_RIGHT:
        setAzimuth(180.0); break;
    case VIEW_FRONT:
        setAzimuth(90.0); break;
    case VIEW_BACK:
        setAzimuth(270.0);
    }

    _fixed = true;
    emit fixedChanged(_fixed, val);
}


void GLWidget::setDir(direction val)
{
    _dir = val;

    emit dirChanged(val);
    update();
}


void GLWidget::setRightHanded(bool val)
{
    _rightHanded = val;

    emit rightHandedChanged(val);
    update();
}


void GLWidget::setShowAxes(bool val)
{
    _showAxes = val;

    emit showAxesChanged(val);
    update();
}


void GLWidget::setShowPoints(bool val)
{
    _showPoints = val;

    update();
}


void GLWidget::initializeDispObject(DisplayObject *obj)
{
    m.lock();
    makeCurrent();
    obj->initialize();
    m.unlock();
}


void GLWidget::matrix(QMatrix4x4 *mvp)
{
    mvp->setToIdentity();

    float aspect = (float) width() / height();
    if (_perspective)
        mvp->perspective(_fov, aspect, 0.01, 100.0);
    else
    {
        float h = _zoom * tan(_fov * 3.14159265 / 360.0);
        mvp->ortho(-aspect * h, aspect * h, -h, h, 0.01, 100.0);
    }

    QVector3D eye = QVector3D(0, (_perspective ? - _zoom : -1.0), 0);
    mvp->lookAt(eye, eye + QVector3D(0, 1, 0), QVector3D(0, 0, 1));
    mvp->rotate(_roll, QVector3D(0, 1, 0));
    mvp->rotate(_inclination, QVector3D(1, 0, 0));
    mvp->rotate(_azimuth, QVector3D(0, 0, 1));
    mvp->scale(1.0/_diameter);
    multiplyDir(mvp);
    mvp->translate(-_lookAt);
}


void GLWidget::axesMatrix(QMatrix4x4 *mvp)
{
    mvp->setToIdentity();

    float aspect = (float) width() / height();
    mvp->translate(QVector3D(1.0 - 0.12/aspect, -0.88, 0));
    mvp->perspective(45.0, aspect, 0.01, 100.0);

    mvp->lookAt(QVector3D(0, 0, 0), QVector3D(0, 1, 0), QVector3D(0, 0, 1));
    mvp->translate(QVector3D(0, 1, 0));
    mvp->rotate(_roll, QVector3D(0, 1, 0));
    mvp->rotate(_inclination, QVector3D(1, 0, 0));
    mvp->rotate(_azimuth, QVector3D(0, 0, 1));
    mvp->scale(0.04);
    multiplyDir(mvp);
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
