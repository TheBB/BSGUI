#include <GoTools/geometry/SplineCurve.h>

#include "DisplayObject.h"

#ifndef CURVE_H
#define CURVE_H

class Curve : public DisplayObject
{
public:
    Curve(Go::SplineCurve *crv);
    ~Curve();

    ObjectType type() { return OT_CURVE; }

    uint nFaces() { return 0; }
    uint nEdges() { return 1; }
    uint nPoints() { return 2; }

private:
    // Pre refinement
    uint nt, ntPts;

    // Refinement
    uint r;

    // Post refinement
    uint n, nPts;

    // Other data
    Go::SplineCurve *crv;
    std::vector<double> knots, params;

    void mkData();
};

#endif /* CURVE_H */
