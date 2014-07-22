#include "DisplayObject.h"


const QVector4D FACE_COLOR_NORMAL  = QVector4D(0.737, 0.929, 1.000, 1);
const QVector4D LINE_COLOR_NORMAL  = QVector4D(0.431, 0.663, 0.749, 0.5);
const QVector4D EDGE_COLOR_NORMAL  = QVector4D(0, 0, 0, 1);
const QVector4D POINT_COLOR_NORMAL = QVector4D(0, 0, 0, 1);

#define LINE_WIDTH 1.1
#define EDGE_WIDTH 2.0
#define POINT_SIZE 8.0


uchar DisplayObject::sColor[3] = {0, 0, 0};


DisplayObject::DisplayObject(int parts)
    : _initialized(false)
    , vertexBufferFaces(QOpenGLBuffer::VertexBuffer)
    , vertexBufferGrid(QOpenGLBuffer::VertexBuffer)
    , faceBuffer(QOpenGLBuffer::IndexBuffer)
    , elementBuffer(QOpenGLBuffer::IndexBuffer)
    , edgeBuffer(QOpenGLBuffer::IndexBuffer)
    , pointBuffer(QOpenGLBuffer::IndexBuffer)
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


void DisplayObject::draw(QMatrix4x4 &mvp, QOpenGLShaderProgram &prog)
{
    if (!_initialized)
        return;

    prog.bind();
    prog.setUniformValue("mvp", mvp);


    // Bind face vertices
    vertexBufferFaces.bind();
    prog.enableAttributeArray("vertexPosition");
    prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    // Draw faces
    faceBuffer.bind();
    prog.setUniformValue("col", FACE_COLOR_NORMAL);
    glDrawElements(GL_QUADS, 4 * faceData.size(), GL_UNSIGNED_INT, 0);


    // Bind grid vertices
    vertexBufferGrid.bind();
    prog.enableAttributeArray("vertexPosition");
    prog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    // Draw elements
    elementBuffer.bind();
    prog.setUniformValue("col", LINE_COLOR_NORMAL);
    glLineWidth(LINE_WIDTH);
    glDrawElements(GL_LINES, 2 * elementData.size(), GL_UNSIGNED_INT, 0);

    // Draw edges
    edgeBuffer.bind();
    prog.setUniformValue("col", EDGE_COLOR_NORMAL);
    glLineWidth(EDGE_WIDTH);
    glDrawElements(GL_LINES, 2 * edgeData.size(), GL_UNSIGNED_INT, 0);

    // Draw points
    pointBuffer.bind();
    prog.setUniformValue("col", POINT_COLOR_NORMAL);
    glPointSize(POINT_SIZE);
    glDrawElements(GL_POINTS, pointData.size(), GL_UNSIGNED_INT, 0);
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
