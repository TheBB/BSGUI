#include "DisplayObject.h"

const QVector3D FACE_COLOR_NORMAL    = QVector3D(0.737, 0.929, 1.000);
const QVector3D LINE_COLOR_NORMAL    = QVector3D(0.431, 0.663, 0.749);
const QVector3D EDGE_COLOR_NORMAL    = QVector3D(0.000, 0.000, 0.000);
const QVector3D POINT_COLOR_NORMAL   = QVector3D(0.000, 0.000, 0.000);

const QVector3D FACE_COLOR_SELECTED  = QVector3D(1.000, 0.867, 0.737);
const QVector3D LINE_COLOR_SELECTED  = QVector3D(0.749, 0.620, 0.431);
const QVector3D EDGE_COLOR_SELECTED  = QVector3D(0.776, 0.478, 0.427);
const QVector3D POINT_COLOR_SELECTED = QVector3D(0.776, 0.478, 0.427);

const QVector3D WHITE = QVector3D(1.0, 1.0, 1.0);


#define LINE_WIDTH 1.1
#define EDGE_WIDTH 2.0
#define POINT_SIZE 10.0


uchar DisplayObject::sColor[3] = {0, 0, 0};


DisplayObject::DisplayObject(int parts)
    : _initialized(false)
    , _visible(true)
    , vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , normalBuffer(QOpenGLBuffer::VertexBuffer)
    , faceBuffer(QOpenGLBuffer::IndexBuffer)
    , elementBuffer(QOpenGLBuffer::IndexBuffer)
    , edgeBuffer(QOpenGLBuffer::IndexBuffer)
    , pointBuffer(QOpenGLBuffer::IndexBuffer)
    , selectedFaces {}
    , selectedEdges {}
    , selectedPoints {}
{
    minColor = sColor[0] + 255*sColor[1] + 255*255*sColor[2];
    maxColor = minColor + parts;

    for (int i = 0; i < 3; i++)
        color[i] = sColor[i];

    incColors(sColor, parts);
}


DisplayObject::~DisplayObject()
{
    if (_initialized)
    {
        _initialized = false;

        vertexBuffer.destroy();
        normalBuffer.destroy();
        faceBuffer.destroy();
        elementBuffer.destroy();
        edgeBuffer.destroy();
        pointBuffer.destroy();
    }
}


void DisplayObject::initialize()
{
    createBuffer(vertexBuffer);
    vertexBuffer.allocate(&vertexData[0], 3 * vertexData.size() * sizeof(float));

    createBuffer(normalBuffer);
    normalBuffer.allocate(&normalData[0], 3 * normalData.size() * sizeof(float));

    createBuffer(faceBuffer);
    faceBuffer.allocate(&faceData[0], 4 * faceData.size() * sizeof(GLuint));

    createBuffer(elementBuffer);
    elementBuffer.allocate(&elementData[0], 2 * elementData.size() * sizeof(GLuint));

    createBuffer(edgeBuffer);
    edgeBuffer.allocate(&edgeData[0], 2 * edgeData.size() * sizeof(GLuint));

    createBuffer(pointBuffer);
    pointBuffer.allocate(&pointData[0], pointData.size() * sizeof(GLuint));

    _initialized = true;
}


void drawCommand(GLenum mode, const std::set<uint> &visible, int n, std::vector<uint> indices)
{
    uint mult = mode == GL_QUADS ? 4 : 2;
    if (visible.size() == n)
        glDrawElements(mode, mult*indices[n], GL_UNSIGNED_INT, 0);
    else
        for (auto i : visible)
        {
            glDrawElements(mode, mult*(indices[i+1] - indices[i]), GL_UNSIGNED_INT,
                           (void *) (mult * indices[i] * sizeof(GLuint)));
        }
}


void drawCommandPts(const std::set<uint> &visible, int n)
{
    if (visible.size() == n)
        glDrawElements(GL_POINTS, n, GL_UNSIGNED_INT, 0);
    else
        for (auto i : visible)
            glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, (void *) (i * sizeof(GLuint)));
}


void sortSelection(const std::set<uint> &selected, const std::set<uint> &visible,
                   std::set<uint> &outSel, std::set<uint> &outUnsel)
{
    outSel.clear();
    outUnsel.clear();

    for (auto f : visible)
        if (selected.find(f) != selected.end())
            outSel.insert(f);
        else
            outUnsel.insert(f);
}


void DisplayObject::draw(QMatrix4x4 &mvp, QOpenGLShaderProgram &prog)
{
    if (!_initialized)
        return;

    std::set<uint> sel, unsel;

    prog.bind();

    bindBuffer(prog, vertexBuffer, "vertexPosition");
    bindBuffer(prog, normalBuffer, "vertexNormal");


    faceBuffer.bind();
    sortSelection(selectedFaces, visibleFaces, sel, unsel);

    setUniforms(prog, mvp, FACE_COLOR_SELECTED, 0.0); drawCommand(GL_QUADS, sel, nFaces(), faceIdxs);
    setUniforms(prog, mvp, FACE_COLOR_NORMAL, 0.0); drawCommand(GL_QUADS, unsel, nFaces(), faceIdxs);


    elementBuffer.bind();
    glLineWidth(LINE_WIDTH);

    setUniforms(prog, mvp, LINE_COLOR_SELECTED, 0.0001); drawCommand(GL_LINES, sel, nFaces(), elementIdxs);
    setUniforms(prog, mvp, LINE_COLOR_NORMAL, 0.0001); drawCommand(GL_LINES, unsel, nFaces(), elementIdxs);
    

    edgeBuffer.bind();
    sortSelection(selectedEdges, visibleEdges, sel, unsel);
    glLineWidth(EDGE_WIDTH);

    setUniforms(prog, mvp, EDGE_COLOR_SELECTED, 0.0002); drawCommand(GL_LINES, sel, nEdges(), edgeIdxs);
    setUniforms(prog, mvp, EDGE_COLOR_NORMAL, 0.0002); drawCommand(GL_LINES, unsel, nEdges(), edgeIdxs);


    pointBuffer.bind();
    sortSelection(selectedPoints, visiblePoints, sel, unsel);
    glPointSize(POINT_SIZE);

    setUniforms(prog, mvp, POINT_COLOR_SELECTED, 0.0); drawCommandPts(sel, nPoints());
    setUniforms(prog, mvp, POINT_COLOR_NORMAL, 0.0); drawCommandPts(unsel, nPoints());
}


void DisplayObject::drawPicking(QMatrix4x4 &mvp, QOpenGLShaderProgram &prog, SelectionMode mode)
{
    if (!_initialized)
        return;

    uchar col[3]; col[0] = color[0]; col[1] = color[1]; col[2] = color[2];

    prog.bind();

    bindBuffer(prog, vertexBuffer, "vertexPosition");
    bindBuffer(prog, normalBuffer, "vertexNormal");


    faceBuffer.bind();
    if (mode == SM_PATCH)
    {
        setUniforms(prog, mvp, col, 0.0); incColors(col, 1);
        drawCommand(GL_QUADS, visibleFaces, nFaces(), faceIdxs);
    }
    else if (mode == SM_FACE)
    {
        for (auto f : visibleFaces)
        {
            setUniforms(prog, mvp, col, 0.0); incColors(col, 1);
            drawCommand(GL_QUADS, {f}, nFaces(), faceIdxs);
        }
    }
    else if (mode == SM_EDGE)
    {
        setUniforms(prog, mvp, WHITE, 0.0);
        drawCommand(GL_QUADS, visibleFaces, nFaces(), faceIdxs);

        edgeBuffer.bind();
        glLineWidth(20 * EDGE_WIDTH);
        for (auto e : visibleEdges)
        {
            setUniforms(prog, mvp, col, 0.0002); incColors(col, 1);
            drawCommand(GL_LINES, {e}, nEdges(), edgeIdxs);
        }
    }
    else if (mode == SM_POINT)
    {
        setUniforms(prog, mvp, WHITE, 0.0);
        drawCommand(GL_QUADS, visibleFaces, nFaces(), faceIdxs);

        pointBuffer.bind();
        glPointSize(POINT_SIZE);
        for (auto p : visiblePoints)
        {
            setUniforms(prog, mvp, col, 0.0002); incColors(col, 1);
            drawCommandPts({p}, nPoints());
        }
    }
}


void DisplayObject::computeBoundingSphere()
{
    QVector3D point = vertexData[0], found;

    farthestPointFrom(point, &found);
    farthestPointFrom(point, &found);

    _center = (point + found) / 2;
    _radius = (point - found).length() / 2;

    ritterSphere();
}


void DisplayObject::farthestPointFrom(QVector3D point, QVector3D *found)
{
    float distance = -1;

    for (auto p : vertexData)
    {
        float _distance = (p - point).length();
        if (_distance > distance)
        {
            distance = _distance;
            *found = p;
        }
    }
}


void DisplayObject::ritterSphere()
{
    for (auto p : vertexData)
    {
        float d = (p - _center).length();
        if (d > _radius)
        {
            _center = ((d + _radius)/2 * _center + (d - _radius)/2 * p) / d;
            _radius = (d + _radius)/2;
        }
    }
}


void DisplayObject::selectionMode(SelectionMode mode, bool conjunction)
{
    switch (mode)
    {
    case SM_PATCH:
        selectObject(!selectedFaces.empty() || !selectedEdges.empty() || !selectedPoints.empty());
        break;

    case SM_FACE:
        if (selectedEdges.empty())
            balloonPointsToEdges(conjunction);
        if (selectedFaces.empty())
            balloonEdgesToFaces(conjunction);
        refreshEdgesFromFaces();
        refreshPointsFromEdges();
        break;

    case SM_EDGE:
        if (selectedFaces.empty() && selectedEdges.empty())
            balloonPointsToEdges(conjunction);
        selectedFaces.clear();
        break;

    case SM_POINT:
        selectedFaces.clear();
        selectedEdges.clear();
        break;
    }
}


void DisplayObject::selectObject(bool selected)
{
    if (selected)
    {
        for (int i = 0; i < nFaces(); i++)
            selectedFaces.insert(i);
        for (int i = 0; i < nEdges(); i++)
            selectedEdges.insert(i);
        for (int i = 0; i < nEdges(); i++)
            selectedPoints.insert(i);
    }
    else
    {
        selectedFaces.clear();
        selectedEdges.clear();
        selectedPoints.clear();
    }
}


void DisplayObject::selectFaces(bool selected, std::set<uint> faces)
{
    if (!selected)
    {
        for (auto f : faces)
            selectedFaces.erase(f);
        refreshEdgesFromFaces();
        refreshPointsFromEdges();

        return;
    }

    selectedFaces.insert(faces.begin(), faces.end());

    std::set<uint> edges;
    for (auto f : faces)
    {
        edges.insert(faceEdgeMap[f].a);
        edges.insert(faceEdgeMap[f].b);
        edges.insert(faceEdgeMap[f].c);
        edges.insert(faceEdgeMap[f].d);
    }

    selectEdges(true, edges);
}


void DisplayObject::selectEdges(bool selected, std::set<uint> edges)
{
    if (!selected)
    {
        for (auto e : edges)
            selectedEdges.erase(e);
        refreshPointsFromEdges();

        return;
    }

    selectedEdges.insert(edges.begin(), edges.end());

    std::set<uint> points;
    for (auto e : edges)
    {
        points.insert(edgePointMap[e].a);
        points.insert(edgePointMap[e].b);
    }

    selectPoints(true, points);
}


void DisplayObject::selectPoints(bool selected, std::set<uint> points)
{
    if (selected)
        selectedPoints.insert(points.begin(), points.end());
    else
        for (auto p : points)
            selectedPoints.erase(p);
}


bool DisplayObject::fullSelection(SelectionMode mode)
{
    if (mode == SM_PATCH)
        return hasSelection();
    else if (mode == SM_FACE)
        return selectedFaces.size() == nFaces();
    else if (mode == SM_EDGE)
        return selectedEdges.size() == nEdges();
    else if (mode == SM_POINT)
        return selectedPoints.size() == nPoints();
}


void DisplayObject::refreshEdgesFromFaces()
{
    selectedEdges.clear();
    for (auto e : selectedFaces)
        selectEdges(true, {faceEdgeMap[e].a, faceEdgeMap[e].b, faceEdgeMap[e].c, faceEdgeMap[e].d});
}


void DisplayObject::refreshPointsFromEdges()
{
    selectedPoints.clear();
    for (auto e : selectedEdges)
        selectPoints(true, {edgePointMap[e].a, edgePointMap[e].b});
}


void DisplayObject::balloonEdgesToFaces(bool conjunction)
{
    std::set<uint> faces;
    for (uint f = 0; f < nFaces(); f++)
        if ((conjunction &&  (selectedEdges.find(faceEdgeMap[f].a) != selectedEdges.end() &&
                              selectedEdges.find(faceEdgeMap[f].b) != selectedEdges.end() &&
                              selectedEdges.find(faceEdgeMap[f].c) != selectedEdges.end() &&
                              selectedEdges.find(faceEdgeMap[f].d) != selectedEdges.end())) ||
            (!conjunction && (selectedEdges.find(faceEdgeMap[f].a) != selectedEdges.end() ||
                              selectedEdges.find(faceEdgeMap[f].b) != selectedEdges.end() ||
                              selectedEdges.find(faceEdgeMap[f].c) != selectedEdges.end() ||
                              selectedEdges.find(faceEdgeMap[f].d) != selectedEdges.end())))
        {
            faces.insert(f);
        }

    selectFaces(true, faces);
}


void DisplayObject::balloonPointsToEdges(bool conjunction)
{
    std::set<uint> edges;
    for (uint e = 0; e < nEdges(); e++)
        if ((conjunction &&  (selectedPoints.find(edgePointMap[e].a) != selectedPoints.end() &&
                              selectedPoints.find(edgePointMap[e].b) != selectedPoints.end())) ||
            (!conjunction && (selectedPoints.find(edgePointMap[e].a) != selectedPoints.end() ||
                              selectedPoints.find(edgePointMap[e].b) != selectedPoints.end())))
        {
            edges.insert(e);
        }

    selectEdges(true, edges);
}


void DisplayObject::createBuffer(QOpenGLBuffer &buffer)
{
    buffer.create();
    buffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    buffer.bind();
}


void DisplayObject::bindBuffer(QOpenGLShaderProgram &prog, QOpenGLBuffer &buffer, const char *attribute)
{
    buffer.bind();
    prog.enableAttributeArray(attribute);
    prog.setAttributeBuffer(attribute, GL_FLOAT, 0, 3);
}


void DisplayObject::setUniforms(QOpenGLShaderProgram &prog, QMatrix4x4 mvp, QVector3D col, float p)
{
    prog.setUniformValue("mvp", mvp);
    prog.setUniformValue("col", col);
    prog.setUniformValue("p", p);
}


void DisplayObject::setUniforms(QOpenGLShaderProgram &prog, QMatrix4x4 mvp, uchar *col, float p)
{
    prog.setUniformValue("mvp", mvp);
    prog.setUniformValue("col", QVector3D((float) col[0]/255, (float) col[1]/255, (float) col[2]/255));
    prog.setUniformValue("p", p);
}


void DisplayObject::incColors(uchar col[3], int num)
{
    for (int i = 0; i < num; i++)
    {
        if (col[0] == 255)
        {
            col[0] = 0;
            if (col[1] == 255)
            {
                col[1] = 0;
                if (col[2] == 255)
                    col[2] = 0;
                else
                    col[2]++;
            }
            else
                col[1]++;
        }
        else
            col[0]++;
    }
}
