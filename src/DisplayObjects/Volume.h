#include "DisplayObject.h"

#ifndef _VOLUME_H_
#define _VOLUME_H_

class Volume : public DisplayObject
{
public:
    Volume(QVector3D center);

    ObjectType type() { return OT_VOLUME; }

    uint nFaces() { return 6; }
    uint nEdges() { return 12; }
    uint nPoints() { return 8; }

private:
    // Pre refinement
    uint ntU, ntV, ntW;
    uint ntPtsU, ntPtsV, ntPtsW, ntPts;
    uint ntElems;

    // Refinement
    uint rU, rV, rW;

    // Post refinement
    uint nU, nV, nW;
    uint nPtsU, nPtsV, nPtsW, nPts, nElems, nElemLines, nLinesUV, nLinesUW, nLinesVW;

    void mkVertexData(QVector3D);
    void mkFaceData();
    void mkElementData();
    void mkEdgeData();
    void mkPointData();

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

    inline uint uvFace(uint i, uint j, bool posW)
    {
        return posW ?
            nU*nV + nU*j + i :
            nU*j + i;
    }

    inline uint uwFace(uint i, uint j, bool posV)
    {
        return posV ?
            2*nU*nV + nU*nW + nU*j + i :
            2*nU*nV + nU*j + i;
    }

    inline uint vwFace(uint i, uint j, bool posU)
    {
        return posU ?
            2*nU*nV + 2*nU*nW + nV*nW + nV*j + i :
            2*nU*nV + 2*nU*nW + nV*j + i;
    }

    inline uint uElmt(uint i, int j, bool posO, bool axV)
    {
        if (axV)
            return 2*nLinesUV + (posO ? nLinesUW : 0) + nU*j + i;
        return (posO ? nLinesUV : 0) + nU*j + i;
    }

    inline uint vElmt(uint i, int j, bool posO, bool axU)
    {
        if (axU)
            return 2*nLinesUV + 2*nLinesUW + (posO ? nLinesVW : 0) + nV*j + i;
        return nU*(ntV-1) + (posO ? nLinesUV : 0) + nV*j + i;
    }

    inline uint wElmt(uint i, int j, bool posO, bool axU)
    {
        if (axU)
            return 2*nLinesUV + 2*nLinesUW + nV*(ntW-1) + (posO ? nLinesVW : 0) + nW*j + i;
        return 2*nLinesUV + nU*(ntW-1) + (posO ? nLinesUW : 0) + nW*j + i;
    }

    inline uint uEdge(uint i, bool posV, bool posW)
    {
        return (posV ? nU : 0) + (posW ? 2*nU : 0) + i;
    }

    inline uint vEdge(uint i, bool posU, bool posW)
    {
        return 4*nU + (posU ? nV : 0) + (posW ? 2*nV : 0) + i;
    }

    inline uint wEdge(uint i, bool posU, bool posV)
    {
        return 4*(nU+nV) + (posU ? nW : 0) + (posV ? 2*nW : 0) + i;
    }
};

#endif /* _VOLUME_H_ */
