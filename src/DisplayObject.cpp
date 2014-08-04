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


uint DisplayObject::nextIndex = 0;
std::map<uint, DisplayObject *> DisplayObject::indexMap;
std::mutex DisplayObject::m;


DisplayObject::DisplayObject()
    : _initialized(false)
    , vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , normalBuffer(QOpenGLBuffer::VertexBuffer)
    , faceBuffer(QOpenGLBuffer::IndexBuffer)
    , elementBuffer(QOpenGLBuffer::IndexBuffer)
    , edgeBuffer(QOpenGLBuffer::IndexBuffer)
    , pointBuffer(QOpenGLBuffer::IndexBuffer)
    , _patch(NULL)
    , selectedFaces {}
    , selectedEdges {}
    , selectedPoints {}
{
    _index = registerObject(this);
}


DisplayObject::~DisplayObject()
{
    deregisterObject(_index);

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
    if (_initialized)
        return;

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


void DisplayObject::draw(QMatrix4x4 &mvp, QOpenGLShaderProgram &prog, bool showPoints)
{
    if (!_initialized)
        return;

    std::set<uint> sel, unsel;

    prog.bind();

    bindBuffer(prog, vertexBuffer, "vertexPosition");
    bindBuffer(prog, normalBuffer, "vertexNormal");


    faceBuffer.bind();
    sortSelection(selectedFaces, visibleFaces, sel, unsel);

    for (auto off : faceOffsets)
    {
        setUniforms(prog, mvp, FACE_COLOR_SELECTED, off);
        drawCommand(GL_QUADS, sel, nFaces(), faceIdxs);
        setUniforms(prog, mvp, FACE_COLOR_NORMAL, off);
        drawCommand(GL_QUADS, unsel, nFaces(), faceIdxs);
    }


    elementBuffer.bind();
    glLineWidth(LINE_WIDTH);

    for (auto off : lineOffsets)
    {
        setUniforms(prog, mvp, LINE_COLOR_SELECTED, off);
        drawCommand(GL_LINES, sel, nFaces(), elementIdxs);
        setUniforms(prog, mvp, LINE_COLOR_NORMAL, off);
        drawCommand(GL_LINES, unsel, nFaces(), elementIdxs);
    }


    edgeBuffer.bind();
    sortSelection(selectedEdges, visibleEdges, sel, unsel);
    glLineWidth(EDGE_WIDTH);

    for (auto off : edgeOffsets)
    {
        setUniforms(prog, mvp, EDGE_COLOR_SELECTED, off);
        drawCommand(GL_LINES, sel, nEdges(), edgeIdxs);
        setUniforms(prog, mvp, EDGE_COLOR_NORMAL, off);
        drawCommand(GL_LINES, unsel, nEdges(), edgeIdxs);
    }


    if (showPoints)
    {
        pointBuffer.bind();
        sortSelection(selectedPoints, visiblePoints, sel, unsel);
        glPointSize(POINT_SIZE);

        for (auto off : pointOffsets)
        {
            setUniforms(prog, mvp, POINT_COLOR_SELECTED, off);
            drawCommandPts(sel, nPoints());
            setUniforms(prog, mvp, POINT_COLOR_NORMAL, off);
            drawCommandPts(unsel, nPoints());
        }
    }
}


void DisplayObject::drawPicking(QMatrix4x4 &mvp, QOpenGLShaderProgram &prog, SelectionMode mode)
{
    if (!_initialized)
        return;

    uint offset = 0;

    prog.bind();

    bindBuffer(prog, vertexBuffer, "vertexPosition");
    bindBuffer(prog, normalBuffer, "vertexNormal");


    faceBuffer.bind();
    if (mode == SM_PATCH)
    {
        if (nFaces() > 0)
            for (auto off : faceOffsets)
            {
                setUniforms(prog, mvp, indexToColor(_index, offset), off);
                drawCommand(GL_QUADS, visibleFaces, nFaces(), faceIdxs);
            }
        else
        {
            edgeBuffer.bind();
            glLineWidth(20 * EDGE_WIDTH);
            for (auto off : edgeOffsets)
            {
                setUniforms(prog, mvp, indexToColor(_index, offset), off);
                drawCommand(GL_LINES, visibleEdges, nEdges(), edgeIdxs);
            }
        }
    }
    else if (mode == SM_FACE)
    {
        for (uint f = 0; f < nFaces(); f++)
        {
            if (visibleFaces.find(f) != visibleFaces.end())
                for (auto off : faceOffsets)
                {
                    setUniforms(prog, mvp, indexToColor(_index, offset), off);
                    drawCommand(GL_QUADS, {f}, nFaces(), faceIdxs);
                }
            offset++;
        }
    }
    else if (mode == SM_EDGE)
    {
        if (nFaces() > 0)
        {
            setUniforms(prog, mvp, WHITE, 0.0);
            drawCommand(GL_QUADS, visibleFaces, nFaces(), faceIdxs);
        }

        edgeBuffer.bind();
        glLineWidth(20 * EDGE_WIDTH);
        for (uint e = 0; e < nEdges(); e++)
        {
            if (visibleEdges.find(e) != visibleEdges.end())
                for (auto off : edgeOffsets)
                {
                    setUniforms(prog, mvp, indexToColor(_index, offset), off);
                    drawCommand(GL_LINES, {e}, nEdges(), edgeIdxs);
                }
            offset++;
        }
    }
    else if (mode == SM_POINT)
    {
        if (nFaces() > 0)
        {
            setUniforms(prog, mvp, WHITE, 0.0);
            drawCommand(GL_QUADS, visibleFaces, nFaces(), faceIdxs);
        }

        pointBuffer.bind();
        glPointSize(POINT_SIZE);
        for (uint p = 0; p < nPoints(); p++)
        {
            if (visiblePoints.find(p) != visiblePoints.end())
                for (auto off : pointOffsets)
                {
                    setUniforms(prog, mvp, indexToColor(_index, offset), off);
                    drawCommandPts({p}, nPoints());
                }
            offset++;
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


void DisplayObject::mkSamples(const std::vector<double> &knots, std::vector<double> &params, uint ref)
{
    params.resize((knots.size() - 1) * ref + 1);

    for (uint i = 0; i < knots.size() - 1; i++)
        for (uint j = 0; j < ref; j++)
            params[i*ref + j] = knots[i] + (double) j / ref * (knots[i+1] - knots[i]);
    params.back() = knots.back();
}


void DisplayObject::selectionMode(SelectionMode mode, bool conjunction)
{
    switch (mode)
    {
    case SM_PATCH:
        selectObject(mode, !selectedFaces.empty() || !selectedEdges.empty() || !selectedPoints.empty());
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


void DisplayObject::selectObject(SelectionMode mode, bool selected)
{
    if (selected)
    {
        if (mode == SM_FACE || mode == SM_PATCH)
        {
            if (nFaces() > 0)
            {
                for (int i = 0; i < nFaces(); i++)
                    selectedFaces.insert(i);
                refreshEdgesFromFaces();
            }
            else
                for (int i = 0; i < nEdges(); i++)
                    selectedEdges.insert(i);
            refreshPointsFromEdges();
        }
        else if (mode == SM_EDGE)
        {
            for (int i = 0; i < nEdges(); i++)
                selectedEdges.insert(i);
            refreshPointsFromEdges();
        }
        else if (mode == SM_POINT)
        {
            for (int i = 0; i < nPoints(); i++)
                selectedPoints.insert(i);
        }
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


void DisplayObject::showSelected(SelectionMode mode, bool visible)
{
    if (mode == SM_PATCH)
    {
        if (!visible)
        {
            visibleFaces = {};
            visibleEdges = {};
            visiblePoints = {};
        }
        else
        {
            for (int i = 0; i < nFaces(); i++)
                visibleFaces.insert(i);
            for (int i = 0; i < nEdges(); i++)
                visibleEdges.insert(i);
            for (int i = 0; i < nPoints(); i++)
                visiblePoints.insert(i);
        }
    }
    else if (mode == SM_FACE)
    {
        if (!visible)
            for (auto f : selectedFaces)
                visibleFaces.erase(f);
        else
            visibleFaces.insert(selectedFaces.begin(), selectedFaces.end());
    }
    else if (mode == SM_EDGE)
    {
        if (!visible)
            for (auto f : selectedEdges)
                visibleEdges.erase(f);
        else
            visibleEdges.insert(selectedEdges.begin(), selectedEdges.end());
    }
    else if (mode == SM_POINT)
    {
        if (!visible)
            for (auto f : selectedPoints)
                visiblePoints.erase(f);
        else
            visiblePoints.insert(selectedPoints.begin(), selectedPoints.end());
    }
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


DisplayObject *DisplayObject::getObject(uint idx)
{
    if (indexMap.find(idx) != indexMap.end())
        return indexMap[idx];
    return NULL;
}


uint DisplayObject::registerObject(DisplayObject *obj)
{
    while (indexMap.find(nextIndex) != indexMap.end())
        nextIndex = (nextIndex + 1) % NUM_INDICES;

    indexMap[nextIndex] = obj;
    return nextIndex;
}


void DisplayObject::deregisterObject(uint index)
{
    indexMap.erase(index);
}


QVector3D DisplayObject::indexToColor(uint index, uint offset)
{
    uint i = COLORS_PER_OBJECT * index + offset;

    uchar red = i % 256; i /= 256;
    uchar green = i % 256; i /= 256;
    uchar blue = i % 256;

    return QVector3D((float) red/255, (float) green/255, (float) blue/255);
}


void DisplayObject::colorToIndex(GLubyte color[3], uint *index, uint *offset)
{
    keyToIndex(colorToKey(color), index, offset);
}


uint DisplayObject::colorToKey(GLubyte color[3])
{
    return color[2]*256*256 + color[1]*256 + color[0];
}


void DisplayObject::keyToIndex(uint key, uint *index, uint *offset)
{
    *index = key / COLORS_PER_OBJECT;
    *offset = key % COLORS_PER_OBJECT;
}
