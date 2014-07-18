#include <QVector4D>


#include <Eigen/Dense>

#include "DispObject.h"


const QVector4D FACE_COLOR_NORMAL = QVector4D(0.737, 0.929, 1.000, 1);
const QVector4D LINE_COLOR_NORMAL = QVector4D(0.431, 0.663, 0.749, 0.5);
const QVector4D FACE_COLOR_SELECTED = QVector4D(1.000, 0.867, 0.737, 1);
const QVector4D LINE_COLOR_SELECTED = QVector4D(0.749, 0.620, 0.431, 0.5);
const QVector4D BLACK = QVector4D(0, 0, 0, 1);

typedef struct { GLuint a, b; } pair;


DispObject::DispObject(QVector3D center)
    : vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , faceBuffer(QOpenGLBuffer::IndexBuffer)
    , boundaryBuffer(QOpenGLBuffer::IndexBuffer)
    , elementBuffer(QOpenGLBuffer::IndexBuffer)
    , visibleFaces {0,1,2,3,4,5}
    , visibleBoundaries {0,1,2,3,4,5}
    , visibleElements {0,1,2,3,4,5}
    , selected(false)
    , _center(center)
    , _initialized(false)
{
    nU =  3; nV =  4; nW =  5;

    nPtsU = nU + 1;
    nPtsV = nV + 1;
    nPtsW = nW + 1;
    nPts = 2*nPtsU*nPtsV + 2*nPtsU*(nPtsW-2) + 2*(nPtsV-2)*(nPtsW-2);

    nElems = 2*nU*nV + 2*nU*nW + 2*nV*nW;

    nLinesUV = 2*nU*nV - nU - nV;
    nLinesUW = 2*nU*nW - nU - nW;
    nLinesVW = 2*nV*nW - nV - nW;

    nElemLines = 2 * (nLinesUV + nLinesUW + nLinesVW);

    faceIdxs[0] = 0;
    faceIdxs[1] = nU*nV;
    faceIdxs[2] = 2*nU*nV;
    faceIdxs[3] = 2*nU*nV + nU*nW;
    faceIdxs[4] = 2*nU*nV + 2*nU*nW;
    faceIdxs[5] = 2*nU*nV + 2*nU*nW + nV*nW;
    faceIdxs[6] = 2*nU*nV + 2*nU*nW + 2*nV*nW;

    boundaryIdxs[0] = 0;
    boundaryIdxs[1] = 2*(nU+nV);
    boundaryIdxs[2] = 4*(nU+nV);
    boundaryIdxs[3] = 4*(nU+nV) + 2*(nU+nW);
    boundaryIdxs[4] = 4*(nU+nV) + 4*(nU+nW);
    boundaryIdxs[5] = 4*(nU+nV) + 4*(nU+nW) + 2*(nV+nW);
    boundaryIdxs[6] = 4*(nU+nV) + 4*(nU+nW) + 4*(nV+nW);

    elementIdxs[0] = 0;
    elementIdxs[1] = nLinesUV;
    elementIdxs[2] = 2*nLinesUV;
    elementIdxs[3] = 2*nLinesUV + nLinesUW;
    elementIdxs[4] = 2*nLinesUV + 2*nLinesUW;
    elementIdxs[5] = 2*nLinesUV + 2*nLinesUW + nLinesVW;
    elementIdxs[6] = 2*nLinesUV + 2*nLinesUW + 2*nLinesVW;

    mkVertexData(_center);
    mkFaceData();
}


DispObject::~DispObject()
{
    vertexBuffer.destroy();
    faceBuffer.destroy();
    boundaryBuffer.destroy();
    elementBuffer.destroy();
}


void DispObject::init()
{
    mkVertexBuffer();
    mkFaceBuffer();
    mkBoundaryBuffer();
    mkElementBuffer();

    _initialized = true;
}


inline void drawCommand(GLenum mode, std::set<uint> &visible, uint *indices)
{
    uint mult = mode == GL_LINES ? 2 : 4;
    if (visible.size() == 6)
        glDrawElements(mode, mult*indices[6], GL_UNSIGNED_INT, 0);
    else
        for (auto i : visible)
            glDrawElements(mode, mult*(indices[i+1]-indices[i]),
                           GL_UNSIGNED_INT, (void *)(mult*indices[i]*sizeof(GLuint)));
}


void DispObject::draw(QMatrix4x4 &proj, QMatrix4x4 &mv, QOpenGLShaderProgram &vprog,
                      QOpenGLShaderProgram &cprog, QOpenGLShaderProgram &lprog)
{
    if (!_initialized)
        return;

    QMatrix4x4 mvp = proj * mv;


    cprog.bind();

    vertexBuffer.bind();
    cprog.enableAttributeArray("vertexPosition");
    cprog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    cprog.setUniformValue("mvp", mvp);

    faceBuffer.bind();
    cprog.setUniformValue("col", selected ? FACE_COLOR_SELECTED : FACE_COLOR_NORMAL );
    drawCommand(GL_QUADS, visibleFaces, faceIdxs);


    lprog.bind();

    vertexBuffer.bind();
    lprog.enableAttributeArray("vertexPosition");
    lprog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    lprog.setUniformValue("proj", proj);
    lprog.setUniformValue("mv", mv);

    elementBuffer.bind();
    lprog.setUniformValue("col", selected ? LINE_COLOR_SELECTED : LINE_COLOR_NORMAL);
    glLineWidth(2.1);
    drawCommand(GL_LINES, visibleElements, elementIdxs);

    boundaryBuffer.bind();
    lprog.setUniformValue("col", BLACK);
    glLineWidth(2.0);
    drawCommand(GL_LINES, visibleBoundaries, boundaryIdxs);
}


void DispObject::intersect(QVector3D &a, QVector3D &b, bool *intersect, float *param)
{
    bool trIntersect = false;
    float trParam = std::numeric_limits<float>::infinity();

    for (auto q : faceData)
    {
        triangleIntersect(a, b, q.a, q.b, q.c, &trIntersect, &trParam);
        if (trIntersect && trParam >= 0.0)
        {
            *intersect = true;
            *param = trParam;
            return;
        }

        triangleIntersect(a, b, q.a, q.c, q.d, &trIntersect, &trParam);
        if (trIntersect && trParam >= 0.0)
        {
            *intersect = true;
            *param = trParam;
            return;
        }
    }

    *intersect = false;
    return;
}


QVector3D DispObject::center()
{
    return _center;
}


void DispObject::createBuffer(QOpenGLBuffer& buffer)
{
    buffer.create();
    buffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    buffer.bind();
}


static inline float lpt(uint i, uint N)
{
    return (float) i/(N-1) * 2.0 - 1.0;
}


void DispObject::mkVertexData(QVector3D center)
{
    uint baseUW = 2*nPtsU*nPtsV;
    uint baseVW = 2*nPtsU*nPtsV + 2*nPtsU*(nPtsW-2);

    vertexData.resize(nPts);

    for (bool b : {true, false})
    {
        for (int i = 0; i < nPtsU; i++)
            for (int j = 0; j < nPtsV; j++)
                vertexData[uvPt(i,j,b)] = center + QVector3D(lpt(i,nPtsU), lpt(j,nPtsV), b ? 1.0 : -1.0);
        for (int i = 0; i < nPtsU; i++)
            for (int j = 1; j < nPtsW - 1; j++)
                vertexData[uwPt(i,j,b)] = center + QVector3D(lpt(i,nPtsU), b ? 1.0 : -1.0, lpt(j,nPtsW));
        for (int i = 1; i < nPtsV - 1; i++)
            for (int j = 1; j < nPtsW - 1; j++)
                vertexData[vwPt(i,j,b)] = center + QVector3D(b ? 1.0 : -1.0, lpt(i,nPtsV), lpt(j,nPtsW));
    }
}


void DispObject::mkFaceData()
{
    faceData.resize(nElems);

    for (bool b : {true, false})
    {
        for (int i = 0; i < nU; i++)
            for (int j = 0; j < nV; j++)
                faceData[uvEl(i,j,b)] = { uvPt(i,j,b), uvPt(i+1,j,b), uvPt(i+1,j+1,b), uvPt(i,j+1,b) };
        for (int i = 0; i < nU; i++)
            for (int j = 0; j < nW; j++)
                faceData[uwEl(i,j,b)] = { uwPt(i,j,b), uwPt(i+1,j,b), uwPt(i+1,j+1,b), uwPt(i,j+1,b) };
        for (int i = 0; i < nV; i++)
            for (int j = 0; j < nW; j++)
                faceData[vwEl(i,j,b)] = { vwPt(i,j,b), vwPt(i+1,j,b), vwPt(i+1,j+1,b), vwPt(i,j+1,b) };
    }
}


void DispObject::mkVertexBuffer()
{
    createBuffer(vertexBuffer);
    vertexBuffer.allocate(&vertexData[0], nPts * 3 * sizeof(float));
}


void DispObject::mkFaceBuffer()
{
    createBuffer(faceBuffer);
    faceBuffer.allocate(&faceData[0], nElems * 4 * sizeof(GLuint));
}


void DispObject::mkBoundaryBuffer()
{
    std::vector<pair> patchBndData(8 * (nU + nV + nW));

    for (bool a : {true, false})
        for (bool b : {true, false})
            for (bool c : {true, false})
            {
                for (int i = 0; i < nU; i++)
                    patchBndData[uPbd(i,a,b,c)] = { uvPt(i, a ? nV : 0, b), uvPt(i+1, a ? nV : 0, b) };
                for (int i = 0; i < nV; i++)
                    patchBndData[vPbd(i,a,b,c)] = { uvPt(a ? nU : 0, i, b), uvPt(a ? nU : 0, i+1, b) };
                for (int i = 0; i < nW; i++)
                    patchBndData[wPbd(i,a,b,c)] = { uwPt(a ? nU : 0, i, b), uwPt(a ? nU : 0, i+1, b) };
            }

    createBuffer(boundaryBuffer);
    boundaryBuffer.allocate(&patchBndData[0], (nU + nV + nW) * 16 * sizeof(GLuint));
}


void DispObject::mkElementBuffer()
{
    std::vector<pair> elementData(nElemLines);

    for (bool a : {false, true})
    {
        for (int i = 0; i < nU; i++)
        {
            for (int j = 1; j < nV; j++)
                elementData[uEll(i, j-1, a, false)] = { uvPt(i, j, a), uvPt(i+1, j, a) };
            for (int j = 1; j < nW; j++)
                elementData[uEll(i, j-1, a, true)] = { uwPt(i, j, a), uwPt(i+1, j, a) };
        }
        for (int i = 0; i < nV; i++)
        {
            for (int j = 1; j < nU; j++)
                elementData[vEll(i, j-1, a, false)] = { uvPt(j, i, a), uvPt(j, i+1, a) };
            for (int j = 1; j < nW; j++)
                elementData[vEll(i, j-1, a, true)] = { vwPt(i, j, a), vwPt(i+1, j, a) };
        }
        for (int i = 0; i < nW; i++)
        {
            for (int j = 1; j < nU; j++)
                elementData[wEll(i, j-1, a, false)] = { uwPt(j, i, a), uwPt(j, i+1, a) };
            for (int j = 1; j < nV; j++)
                elementData[wEll(i, j-1, a, true)] = { vwPt(j, i, a), vwPt(j, i+1, a) };
        }
    }

    createBuffer(elementBuffer);
    elementBuffer.allocate(&elementData[0], 2 * nElemLines * sizeof(GLuint));
}


void DispObject::triangleIntersect(QVector3D &a, QVector3D &b, uint i, uint j, uint k,
                                   bool *intersect, float *param)
{
    Eigen::Matrix3f mx;
    mx(0,0) = vertexData[i].x() - vertexData[k].x();
    mx(1,0) = vertexData[i].y() - vertexData[k].y();
    mx(2,0) = vertexData[i].z() - vertexData[k].z();
    mx(0,1) = vertexData[j].x() - vertexData[k].x();
    mx(1,1) = vertexData[j].y() - vertexData[k].y();
    mx(2,1) = vertexData[j].z() - vertexData[k].z();
    mx(0,2) = a.x() - b.x();
    mx(1,2) = a.y() - b.y();
    mx(2,2) = a.z() - b.z();

    if (abs(mx.determinant()) < 1e-4)
        return;

    Eigen::Vector3f vec;
    vec(0) = a.x() - vertexData[k].x();
    vec(1) = a.y() - vertexData[k].y();
    vec(2) = a.z() - vertexData[k].z();

    Eigen::Vector3f sol = mx.colPivHouseholderQr().solve(vec);

    if (sol(0) >= 0.0 && sol(1) >= 0.0 && sol(0) + sol(1) <= 1.0 && sol(2) >= 0.0)
    {
        *intersect = true;
        *param = sol(2);
    }
}
