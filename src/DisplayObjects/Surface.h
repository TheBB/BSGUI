#include <GoTools/geometry/SplineSurface.h>

#include "DisplayObject.h"

#ifndef SURFACE_H
#define SURFACE_H

class Surface : public DisplayObject
{
public:
    Surface(Go::SplineSurface *srf);
    ~Surface();

    ObjectType type() { return OT_SURFACE; }

    uint nFaces() { return 1; }
    uint nEdges() { return 4; }
    uint nPoints() { return 4; }

private:
    // Pre refinement
    uint ntU, ntV;
    uint ntPtsU, ntPtsV, ntPts;
    uint ntElems;

    // Refinement
    uint rU, rV;

    // Post refinement
    uint nU, nV;
    uint nPtsU, nPtsV, nPts, nElems, nElemLines;

    // Other data
    Go::SplineSurface *srf;
    std::vector<double> uKnots, vKnots;
    std::vector<double> uParams, vParams;

    void mkVertexData();
    void mkFaceData();
    void mkElementData();
    void mkEdgeData();
    void mkPointData();

    inline uint pt(uint i, uint j) { return nPtsU*j + i; }
    inline uint face(uint i, uint j) { return nU*j + i; }
    inline uint uElmt(uint i, uint j) { return nU*j + i; }
    inline uint vElmt(uint i, uint j) { return nU*(ntV-1) + nV*j + i; }
    inline uint uEdge(uint i, bool posV) { return (posV ? nU : 0) + i; }
    inline uint vEdge(uint i, bool posU) { return 2*nU + (posU ? nV : 0) + i; }
};

#endif /* SURFACE_H */
