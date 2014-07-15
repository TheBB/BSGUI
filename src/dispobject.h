#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

#ifndef DISPOBJECT_H
#define DISPOBJECT_H

typedef unsigned short ushort;
typedef unsigned int uint;

class DispObject
{
public:
    DispObject();
    virtual ~DispObject();

    void init();
    void draw(QMatrix4x4&, QMatrix4x4&, QOpenGLShaderProgram&, QOpenGLShaderProgram&, QOpenGLShaderProgram&);

private:
    ushort nU, nV, nW;
    uint nPtsU, nPtsV, nPtsW, nPts, nElems, nElemLines;

    QOpenGLBuffer vertexBuffer;
    QOpenGLBuffer colorBuffer;
    QOpenGLBuffer faceBuffer;
    QOpenGLBuffer patchBndBuffer;
    QOpenGLBuffer elemBuffer;

    inline uint uvPt(uint i, uint j, bool posW)
    {
        return posW ?
            nPtsU*nPtsV + nPtsU*j + i :
            nPtsU*j + i;
    }

    inline uint uwPt(uint i, uint j, bool posV)
    {
        if (j == 0 || j == nW)
            return uvPt(i, posV ? nV : 0, j != 0);
        return posV ?
            2*nPtsU*nPtsV + nPtsU*(nPtsW-2) + nPtsU*(j-1) + i :
            2*nPtsU*nPtsV + nPtsU*(j-1) + i;
    }

    inline uint vwPt(uint i, uint j, bool posU)
    {
        if (j == 0 || j == nW)
            return uvPt(posU ? nU : 0, i, j != 0);
        if (i == 0 || i == nV)
            return uwPt(posU ? nU : 0, j, i != 0);
        return posU ?
            2*nPtsU*nPtsV + 2*nPtsU*(nPtsW-2) + (nPtsV-2)*(nPtsW-2) + (nPtsV-2)*(j-1) + (i-1) :
            2*nPtsU*nPtsV + 2*nPtsU*(nPtsW-2) + (nPtsV-2)*(j-1) + (i-1);
    }

    inline uint uvEl(uint i, uint j, bool posW)
    {
        return posW ?
            nU*nV + nU*j + i :
            nU*j + i;
    }

    inline uint uwEl(uint i, uint j, bool posV)
    {
        return posV ?
            2*nU*nV + nU*nW + nU*j + i :
            2*nU*nV + nU*j + i;
    }

    inline int vwEl(uint i, uint j, bool posU)
    {
        return posU ?
            2*nU*nV + 2*nU*nW + nV*nW + nV*j + i :
            2*nU*nV + 2*nU*nW + nV*j + i;
    }
};

#endif /* DISPOBJECT_H */
