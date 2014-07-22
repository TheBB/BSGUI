#include <algorithm>

#include "DisplayObject.h"

const QVector4D FACE_COLOR_NORMAL    = QVector4D(0.737, 0.929, 1.000, 1.0);
const QVector4D LINE_COLOR_NORMAL    = QVector4D(0.431, 0.663, 0.749, 0.5);
const QVector4D EDGE_COLOR_NORMAL    = QVector4D(0.000, 0.000, 0.000, 1.0);
const QVector4D POINT_COLOR_NORMAL   = QVector4D(0.000, 0.000, 0.000, 1.0);

const QVector4D FACE_COLOR_SELECTED  = QVector4D(1.000, 0.867, 0.737, 1.0);
const QVector4D LINE_COLOR_SELECTED  = QVector4D(0.749, 0.620, 0.431, 0.5);
const QVector4D EDGE_COLOR_SELECTED  = QVector4D(0.776, 0.478, 0.427, 1.0);
const QVector4D POINT_COLOR_SELECTED = QVector4D(0.776, 0.478, 0.427, 1.0);


#define LINE_WIDTH 1.1
#define EDGE_WIDTH 2.0
#define POINT_SIZE 10.0


uchar DisplayObject::sColor[3] = {0, 0, 0};


DisplayObject::DisplayObject(int parts)
    : _initialized(false)
    , _selectionMode(SM_NONE)
    , _visible(true)
    , vertexBufferFaces(QOpenGLBuffer::VertexBuffer)
    , vertexBufferGrid(QOpenGLBuffer::VertexBuffer)
    , faceBuffer(QOpenGLBuffer::IndexBuffer)
    , elementBuffer(QOpenGLBuffer::IndexBuffer)
    , edgeBuffer(QOpenGLBuffer::IndexBuffer)
    , pointBuffer(QOpenGLBuffer::IndexBuffer)
    , selectedFaces {}
    , selectedEdges {}
    , selectedPoints {}
{
    for (int i = 0; i < 3; i++)
        color[i] = sColor[i];

    incColors(sColor, parts);
}


DisplayObject::~DisplayObject()
{
    if (_initialized)
    {
        _initialized = false;

        vertexBufferFaces.destroy();
        vertexBufferGrid.destroy();
        faceBuffer.destroy();
        elementBuffer.destroy();
        edgeBuffer.destroy();
        pointBuffer.destroy();
    }
}


void DisplayObject::initialize()
{
    createBuffer(vertexBufferFaces);
    vertexBufferFaces.allocate(&vertexDataFaces[0], 3 * vertexDataFaces.size() * sizeof(float));

    createBuffer(vertexBufferGrid);
    vertexBufferGrid.allocate(&vertexDataGrid[0], 3 * vertexDataGrid.size() * sizeof(float));

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
    prog.setUniformValue("mvp", mvp);

    // Bind face vertices
    vertexBufferFaces.bind();
    prog.enableAttributeArray("vertexPosition");
    prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    // Draw faces
    faceBuffer.bind();
    sortSelection(selectedFaces, visibleFaces, sel, unsel);
    prog.setUniformValue("col", FACE_COLOR_SELECTED);
    drawCommand(GL_QUADS, sel, nFaces(), faceIdxs);
    prog.setUniformValue("col", FACE_COLOR_NORMAL);
    drawCommand(GL_QUADS, unsel, nFaces(), faceIdxs);


    // Bind grid vertices
    vertexBufferGrid.bind();
    prog.enableAttributeArray("vertexPosition");
    prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    // Draw elements
    elementBuffer.bind();
    glLineWidth(LINE_WIDTH);
    sortSelection(selectedFaces, visibleFaces, sel, unsel);
    prog.setUniformValue("col", LINE_COLOR_SELECTED);
    drawCommand(GL_LINES, sel, nFaces(), elementIdxs);
    prog.setUniformValue("col", LINE_COLOR_NORMAL);
    drawCommand(GL_LINES, unsel, nFaces(), elementIdxs);

    // Draw edges
    edgeBuffer.bind();
    glLineWidth(EDGE_WIDTH);
    sortSelection(selectedEdges, visibleEdges, sel, unsel);
    prog.setUniformValue("col", EDGE_COLOR_SELECTED);
    drawCommand(GL_LINES, sel, nEdges(), edgeIdxs);
    prog.setUniformValue("col", EDGE_COLOR_NORMAL);
    drawCommand(GL_LINES, unsel, nEdges(), edgeIdxs);

    // Draw points
    pointBuffer.bind();
    glPointSize(POINT_SIZE);
    sortSelection(selectedPoints, visiblePoints, sel, unsel);
    prog.setUniformValue("col", POINT_COLOR_SELECTED);
    drawCommandPts(sel, nPoints());
    prog.setUniformValue("col", POINT_COLOR_NORMAL);
    drawCommandPts(unsel, nPoints());
}


void DisplayObject::drawPicking(QMatrix4x4 &mvp, QOpenGLShaderProgram &prog)
{
}


void DisplayObject::computeBoundingSphere()
{
    QVector3D point = vertexDataGrid[0], found;

    farthestPointFrom(point, &found);
    farthestPointFrom(point, &found);

    _center = (point + found) / 2;
    _radius = (point - found).length() / 2;

    ritterSphere();
}


void DisplayObject::farthestPointFrom(QVector3D point, QVector3D *found)
{
    float distance = -1;

    for (auto p : vertexDataGrid)
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
    for (auto p : vertexDataGrid)
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
    if (_selectionMode == mode)
        return;

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
        selectedFaces = visibleFaces;
        selectedEdges = visibleEdges;
        selectedPoints = visiblePoints;
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

    std::set<uint> isct;
    set_intersection(faces.begin(), faces.end(), visibleFaces.begin(), visibleFaces.end(),
                     std::inserter(isct, isct.begin()));
    selectedFaces.insert(isct.begin(), isct.end());

    std::set<uint> edges;
    for (auto f : isct)
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

    std::set<uint> isct;
    set_intersection(edges.begin(), edges.end(), visibleEdges.begin(), visibleEdges.end(),
                     std::inserter(isct, isct.begin()));
    selectedEdges.insert(isct.begin(), isct.end());

    std::set<uint> points;
    for (auto e : isct)
    {
        points.insert(edgePointMap[e].a);
        points.insert(edgePointMap[e].b);
    }

    selectPoints(true, points);
}


void DisplayObject::selectPoints(bool selected, std::set<uint> points)
{
    if (selected)
    {
        std::set<uint> isct;
        set_intersection(points.begin(), points.end(), visiblePoints.begin(), visiblePoints.end(),
                         std::inserter(isct, isct.begin()));
        selectedPoints.insert(isct.begin(), isct.end());
    }
    else
        for (auto p : points)
            selectedPoints.erase(p);
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
