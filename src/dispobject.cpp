#include <QVector4D>

#include "dispobject.h"


typedef struct { GLuint a, b, c, d; } quad;
typedef struct { GLuint a, b; } pair;



DispObject::DispObject() : vertexBuffer(QOpenGLBuffer::VertexBuffer),
                           faceBuffer(QOpenGLBuffer::IndexBuffer),
                           patchBndBuffer(QOpenGLBuffer::IndexBuffer),
                           elemBuffer(QOpenGLBuffer::IndexBuffer)
{
}


DispObject::~DispObject()
{
    vertexBuffer.destroy();
    faceBuffer.destroy();
    patchBndBuffer.destroy();
}


void DispObject::init()
{
    nU =  3; nV =  4; nW =  5;

    nPtsU = nU + 1;
    nPtsV = nV + 1;
    nPtsW = nW + 1;
    nPts = 2*nPtsU*nPtsV + 2*nPtsU*(nPtsW-2) + 2*(nPtsV-2)*(nPtsW-2);

    nElems = 2*nU*nV + 2*nU*nW + 2*nV*nW;

    mkVertexBuffer();
    mkFaceBuffer();

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

    patchBndBuffer.create();
    patchBndBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    patchBndBuffer.bind();
    patchBndBuffer.allocate(&patchBndData[0], (nU + nV + nW) * 16 * sizeof(GLuint));

    nElemLines = 2 * (2*nU*nV - nU - nV +
                      2*nU*nW - nU - nW +
                      2*nV*nW - nV - nW);
    std::vector<GLuint> elemData(2 * nElemLines);

    uint base = 0;

    for (int i = 0; i < nU; i++)
        for (int j = 1; j < nV; j++)
        {
            elemData[4*(base + nU*(j-1) + i) + 0] = uvPt(i,j,false);
            elemData[4*(base + nU*(j-1) + i) + 1] = uvPt(i+1,j,false);
            elemData[4*(base + nU*(j-1) + i) + 2] = uvPt(i,j,true);
            elemData[4*(base + nU*(j-1) + i) + 3] = uvPt(i+1,j,true);
        }

    base = nU * (nV-1);

    for (int i = 1; i < nU; i++)
        for (int j = 0; j < nV; j++)
        {
            elemData[4*(base + (nU-1)*j + i-1) + 0] = uvPt(i,j,false);
            elemData[4*(base + (nU-1)*j + i-1) + 1] = uvPt(i,j+1,false);
            elemData[4*(base + (nU-1)*j + i-1) + 2] = uvPt(i,j,true);
            elemData[4*(base + (nU-1)*j + i-1) + 3] = uvPt(i,j+1,true);
        }

    base += (nU-1) * nV;

    for (int i = 0; i < nU; i++)

        for (int j = 1; j < nW; j++)
        {
            elemData[4*(base + nU*(j-1) + i) + 0] = uwPt(i,j,false);
            elemData[4*(base + nU*(j-1) + i) + 1] = uwPt(i+1,j,false);
            elemData[4*(base + nU*(j-1) + i) + 2] = uwPt(i,j,true);
            elemData[4*(base + nU*(j-1) + i) + 3] = uwPt(i+1,j,true);
        }

    base += nU * (nW-1);

    for (int i = 1; i < nU; i++)
        for (int j = 0; j < nW; j++)
        {
            elemData[4*(base + (nU-1)*j + i-1) + 0] = uwPt(i,j,false);
            elemData[4*(base + (nU-1)*j + i-1) + 1] = uwPt(i,j+1,false);
            elemData[4*(base + (nU-1)*j + i-1) + 2] = uwPt(i,j,true);
            elemData[4*(base + (nU-1)*j + i-1) + 3] = uwPt(i,j+1,true);
        }

    base += (nU-1) * nW;

    for (int i = 0; i < nV; i++)
        for (int j = 1; j < nW; j++)
        {
            elemData[4*(base + nV*(j-1) + i) + 0] = vwPt(i,j,false);
            elemData[4*(base + nV*(j-1) + i) + 1] = vwPt(i+1,j,false);
            elemData[4*(base + nV*(j-1) + i) + 2] = vwPt(i,j,true);
            elemData[4*(base + nV*(j-1) + i) + 3] = vwPt(i+1,j,true);
        }

    base += nV * (nW-1);

    for (int i = 1; i < nV; i++)
        for (int j = 0; j < nW; j++)
        {
            elemData[4*(base + (nV-1)*j + i-1) + 0] = vwPt(i,j,false);
            elemData[4*(base + (nV-1)*j + i-1) + 1] = vwPt(i,j+1,false);
            elemData[4*(base + (nV-1)*j + i-1) + 2] = vwPt(i,j,true);
            elemData[4*(base + (nV-1)*j + i-1) + 3] = vwPt(i,j+1,true);
        }

    elemBuffer.create();
    elemBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    elemBuffer.bind();
    elemBuffer.allocate(&elemData[0], 2 * nElemLines * sizeof(GLuint));
}


static inline float lpt(uint i, uint N)
{
    return (float) i/(N-1) * 2.0 - 1.0;
}


void DispObject::mkVertexBuffer()
{
    uint baseUW = 2*nPtsU*nPtsV;
    uint baseVW = 2*nPtsU*nPtsV + 2*nPtsU*(nPtsW-2);

    std::vector<QVector3D> vertexData(nPts);

    for (bool b : {true, false})
    {
        for (int i = 0; i < nPtsU; i++)
            for (int j = 0; j < nPtsV; j++)
                vertexData[uvPt(i,j,b)] = QVector3D(lpt(i,nPtsU), lpt(j,nPtsV), b ? 1.0 : -1.0);

        for (int i = 0; i < nPtsU; i++)
            for (int j = 1; j < nPtsW - 1; j++)
                vertexData[uwPt(i,j,b)] = QVector3D(lpt(i,nPtsU), b ? 1.0 : -1.0, lpt(j,nPtsW));

        for (int i = 1; i < nPtsV - 1; i++)
            for (int j = 1; j < nPtsW - 1; j++)
                vertexData[vwPt(i,j,b)] = QVector3D(b ? 1.0 : -1.0, lpt(i,nPtsV), lpt(j,nPtsW));
    }

    vertexBuffer.create();
    vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexBuffer.bind();
    vertexBuffer.allocate(&vertexData[0], nPts * 3 * sizeof(float));
}


void DispObject::mkFaceBuffer()
{
    std::vector<quad> faceData(nElems);

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

    faceBuffer.create();
    faceBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    faceBuffer.bind();
    faceBuffer.allocate(&faceData[0], nElems * 4 * sizeof(GLuint));
}


void DispObject::draw(QMatrix4x4 &proj, QMatrix4x4 &mv, QOpenGLShaderProgram &vprog,
                      QOpenGLShaderProgram &cprog, QOpenGLShaderProgram &lprog)
{
    QMatrix4x4 mvp = proj * mv;
    

    cprog.bind();

    vertexBuffer.bind();
    cprog.enableAttributeArray("vertexPosition");
    cprog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    cprog.setUniformValue("mvp", mvp);

    faceBuffer.bind();
    cprog.setUniformValue("col", QVector4D(0.737, 0.929, 1.000, 1.0));
    glDrawElements(GL_QUADS, nElems * 4, GL_UNSIGNED_INT, 0);


    lprog.bind();

    vertexBuffer.bind();
    lprog.enableAttributeArray("vertexPosition");
    lprog.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

    lprog.setUniformValue("proj", proj);
    lprog.setUniformValue("mv", mv);

    elemBuffer.bind();
    lprog.setUniformValue("col", QVector4D(0.431, 0.663, 0.749, 1.0));
    glLineWidth(1.0);
    glDrawElements(GL_LINES, 2 * nElemLines, GL_UNSIGNED_INT, 0);

    patchBndBuffer.bind();
    lprog.setUniformValue("col", QVector4D(0.0, 0.0, 0.0, 1.0));
    glLineWidth(2.0);
    glDrawElements(GL_LINES, 16 * (nU + nV + nW), GL_UNSIGNED_INT, 0);
}
