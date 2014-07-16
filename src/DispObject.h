#include <set>

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QVector4D>

#ifndef DISPOBJECT_H
#define DISPOBJECT_H

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef struct { GLuint a, b, c, d; } quad;

class DispObject
{
public:
    DispObject();
    virtual ~DispObject();

    void init(QVector3D);
    void draw(QMatrix4x4&, QMatrix4x4&, QOpenGLShaderProgram&, QOpenGLShaderProgram&, QOpenGLShaderProgram&);
    void intersect(QVector3D &a, QVector3D &b, bool *intersect, float *param);

    bool selected;

private:
    ushort nU, nV, nW;
    uint nPtsU, nPtsV, nPtsW, nPts, nElems, nElemLines, nLinesUV, nLinesUW, nLinesVW;

    std::vector<QVector3D> vertexData;
    std::vector<quad> faceData;

    QOpenGLBuffer vertexBuffer;
    QOpenGLBuffer faceBuffer;
    QOpenGLBuffer boundaryBuffer;
    QOpenGLBuffer elementBuffer;

    std::set<uint> visibleFaces;
    std::set<uint> visibleBoundaries;
    std::set<uint> visibleElements;
    uint faceIdxs[7];
    uint boundaryIdxs[7];
    uint elementIdxs[7];

    static void createBuffer(QOpenGLBuffer&);
    void mkVertexBuffer(QVector3D);
    void mkFaceBuffer();
    void mkBoundaryBuffer();
    void mkElementBuffer();

    void triangleIntersect(QVector3D &, QVector3D &, uint, uint, uint, bool *, float *);

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

    inline uint vwEl(uint i, uint j, bool posU)
    {
        return posU ?
            2*nU*nV + 2*nU*nW + nV*nW + nV*j + i :
            2*nU*nV + 2*nU*nW + nV*j + i;
    }

    inline uint uPbd(uint i, bool posV, bool posW, bool axV)
    {
        if (axV)
            return 4*(nU+nV) + (posV ? 2*(nU+nW) : 0) + (posW ? nU : 0) + i;
        return (posV ? nU : 0) + (posW ? 2*(nU+nV) : 0) + i;
    }

    inline uint vPbd(uint i, bool posU, bool posW, bool axU)
    {
        if (axU)
            return 4*(nU+nV) + 4*(nU+nW) + (posU ? 2*(nV+nW) : 0) + (posW ? nV : 0) + i;
        return 2*nU + (posU ? nV : 0) + (posW ? 2*(nU+nV) : 0) + i;
    }

    inline uint wPbd(uint i, bool posU, bool posV, bool axU)
    {
        if (axU)
            return 4*(nU+nV) + 4*(nU+nW) + 2*nV + (posU ? 2*(nV+nW) : 0) + (posV ? nW : 0) + i;
        return 4*(nU+nV) + 2*nU + (posU ? nW : 0) + (posV ? 2*(nU+nW) : 0) + i;
    }

    inline uint uEll(uint i, int j, bool posO, bool axV)
    {
        if (axV)
            return 2*nLinesUV + (posO ? nLinesUW : 0) + nU*j + i;
        return (posO ? nLinesUV : 0) + nU*j + i;
    }

    inline uint vEll(uint i, int j, bool posO, bool axU)
    {
        if (axU)
            return 2*nLinesUV + 2*nLinesUW + (posO ? nLinesVW : 0) + nV*j + i;
        return nU*(nV-1) + (posO ? nLinesUV : 0) + nV*j + i;
    }

    inline uint wEll(uint i, int j, bool posO, bool axU)
    {
        if (axU)
            return 2*nLinesUV + 2*nLinesUW + nV*(nW-1) + (posO ? nLinesVW : 0) + nW*j + i;
        return 2*nLinesUV + nU*(nW-1) + (posO ? nLinesUW : 0) + nW*j + i;
    }
};

#endif /* DISPOBJECT_H */
