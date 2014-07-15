#include <QVector4D>

#include "dispobject.h"


DispObject::DispObject() : vertexBuffer(QOpenGLBuffer::VertexBuffer),
                           colorBuffer(QOpenGLBuffer::VertexBuffer),
                           faceBuffer(QOpenGLBuffer::IndexBuffer),
                           patchBndBuffer(QOpenGLBuffer::IndexBuffer),
                           elemBuffer(QOpenGLBuffer::IndexBuffer)
{
}


DispObject::~DispObject()
{
    vertexBuffer.destroy();
    colorBuffer.destroy();
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
    uint baseUW = 2*nPtsU*nPtsV;
    uint baseVW = 2*nPtsU*nPtsV + 2*nPtsU*(nPtsW-2);

    std::vector<QVector3D> vertexData(nPts);

    for (int i = 0; i < nPtsU; i++)
        for (int j = 0; j < nPtsV; j++)
        {
            vertexData[uvPt(i,j,false)] = 
                QVector3D((float)i/(nPtsU-1)*2.0 - 1.0, (float)j/(nPtsV-1)*2.0 - 1.0, -1.0);
            vertexData[uvPt(i,j,true)] =
                QVector3D((float)i/(nPtsU-1)*2.0 - 1.0, (float)j/(nPtsV-1)*2.0 - 1.0, 1.0);
        }

    for (int i = 0; i < nPtsU; i++)
        for (int j = 1; j < nPtsW - 1; j++)
        {
            vertexData[uwPt(i,j,false)] =
                QVector3D((float)i/(nPtsU-1)*2.0 - 1.0, -1.0, (float)j/(nPtsW-1)*2.0 - 1.0);
            vertexData[uwPt(i,j,true)] =
                QVector3D((float)i/(nPtsU-1)*2.0 - 1.0, 1.0, (float)j/(nPtsW-1)*2.0 - 1.0);
        }

    for (int i = 1; i < nPtsV - 1; i++)
        for (int j = 1; j < nPtsW - 1; j++)
        {
            vertexData[vwPt(i,j,false)] =
                QVector3D(-1.0, (float)i/(nPtsV-1)*2.0 - 1.0, (float)j/(nPtsW-1)*2.0 - 1.0);
            vertexData[vwPt(i,j,true)] =
                QVector3D(1.0, (float)i/(nPtsV-1)*2.0 - 1.0, (float)j/(nPtsW-1)*2.0 - 1.0);
        }

    vertexBuffer.create();
    vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexBuffer.bind();
    vertexBuffer.allocate(&vertexData[0], nPts * 3 * sizeof(float));

    nElems = 2*nU*nV + 2*nU*nW + 2*nV*nW;

    std::vector<GLuint> faceData(4 * nElems);

    for (int i = 0; i < nU; i++)
        for (int j = 0; j < nV; j++)
        {
            faceData[4*uvEl(i,j,false) + 0] = uvPt(i,j,false);
            faceData[4*uvEl(i,j,false) + 1] = uvPt(i+1,j,false);
            faceData[4*uvEl(i,j,false) + 2] = uvPt(i+1,j+1,false);
            faceData[4*uvEl(i,j,false) + 3] = uvPt(i,j+1,false);
            faceData[4*uvEl(i,j,true)  + 0] = uvPt(i,j,true);
            faceData[4*uvEl(i,j,true)  + 1] = uvPt(i+1,j,true);
            faceData[4*uvEl(i,j,true)  + 2] = uvPt(i+1,j+1,true);
            faceData[4*uvEl(i,j,true)  + 3] = uvPt(i,j+1,true);
        }

    for (int i = 0; i < nU; i++)
        for (int j = 0; j < nW; j++)
        {
            faceData[4*uwEl(i,j,false) + 0] = uwPt(i,j,false);
            faceData[4*uwEl(i,j,false) + 1] = uwPt(i+1,j,false);
            faceData[4*uwEl(i,j,false) + 2] = uwPt(i+1,j+1,false);
            faceData[4*uwEl(i,j,false) + 3] = uwPt(i,j+1,false);
            faceData[4*uwEl(i,j,true)  + 0] = uwPt(i,j,true);
            faceData[4*uwEl(i,j,true)  + 1] = uwPt(i+1,j,true);
            faceData[4*uwEl(i,j,true)  + 2] = uwPt(i+1,j+1,true);
            faceData[4*uwEl(i,j,true)  + 3] = uwPt(i,j+1,true);
        }

    for (int i = 0; i < nV; i++)
        for (int j = 0; j < nW; j++)
        {
            faceData[4*vwEl(i,j,false) + 0] = vwPt(i,j,false);
            faceData[4*vwEl(i,j,false) + 1] = vwPt(i+1,j,false);
            faceData[4*vwEl(i,j,false) + 2] = vwPt(i+1,j+1,false);
            faceData[4*vwEl(i,j,false) + 3] = vwPt(i,j+1,false);
            faceData[4*vwEl(i,j,true)  + 0] = vwPt(i,j,true);
            faceData[4*vwEl(i,j,true)  + 1] = vwPt(i+1,j,true);
            faceData[4*vwEl(i,j,true)  + 2] = vwPt(i+1,j+1,true);
            faceData[4*vwEl(i,j,true)  + 3] = vwPt(i,j+1,true);
        }

    faceBuffer.create();
    faceBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    faceBuffer.bind();
    faceBuffer.allocate(&faceData[0], nElems * 4 * sizeof(GLuint));

    std::vector<GLuint> patchBndData(8 * (nU + nV + nW));

    for (int i = 0; i < nU; i++)
    {
        patchBndData[8*i + 0] = uvPt(i,0,false);
        patchBndData[8*i + 1] = uvPt(i+1,0,false);
        patchBndData[8*i + 2] = uvPt(i,nV,false);
        patchBndData[8*i + 3] = uvPt(i+1,nV,false);
        patchBndData[8*i + 4] = uvPt(i,0,true);
        patchBndData[8*i + 5] = uvPt(i+1,0,true);
        patchBndData[8*i + 6] = uvPt(i,nV,true);
        patchBndData[8*i + 7] = uvPt(i+1,nV,true);
    }

    for (int i = 0; i < nV; i++)
    {
        patchBndData[8*nU + 8*i + 0] = uvPt(0,i,false);
        patchBndData[8*nU + 8*i + 1] = uvPt(0,i+1,false);
        patchBndData[8*nU + 8*i + 2] = uvPt(nU,i,false);
        patchBndData[8*nU + 8*i + 3] = uvPt(nU,i+1,false);
        patchBndData[8*nU + 8*i + 4] = uvPt(0,i,true);
        patchBndData[8*nU + 8*i + 5] = uvPt(0,i+1,true);
        patchBndData[8*nU + 8*i + 6] = uvPt(nU,i,true);
        patchBndData[8*nU + 8*i + 7] = uvPt(nU,i+1,true);
    }

    for (int i = 0; i < nW; i++)
    {
        patchBndData[8*(nU+nV) + 8*i + 0] = uwPt(0,i,false);
        patchBndData[8*(nU+nV) + 8*i + 1] = uwPt(0,i+1,false);
        patchBndData[8*(nU+nV) + 8*i + 2] = uwPt(nU,i,false);
        patchBndData[8*(nU+nV) + 8*i + 3] = uwPt(nU,i+1,false);
        patchBndData[8*(nU+nV) + 8*i + 4] = uwPt(0,i,true);
        patchBndData[8*(nU+nV) + 8*i + 5] = uwPt(0,i+1,true);
        patchBndData[8*(nU+nV) + 8*i + 6] = uwPt(nU,i,true);
        patchBndData[8*(nU+nV) + 8*i + 7] = uwPt(nU,i+1,true);
    }

    patchBndBuffer.create();
    patchBndBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    patchBndBuffer.bind();
    patchBndBuffer.allocate(&patchBndData[0], (nU + nV + nW) * 8 * sizeof(GLuint));

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
    glDrawElements(GL_LINES, (nU + nV + nW) * 8, GL_UNSIGNED_INT, 0);
}
