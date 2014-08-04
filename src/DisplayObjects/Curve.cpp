#include "DisplayObjects/Curve.h"


Curve::Curve(Go::SplineCurve *c)
    : DisplayObject()
    , crv(c)
{
    c->basis().knotsSimple(knots);


    // Pre refinement
    nt = knots.size() - 1;
    ntPts = nt + 1;


    // Refinement
    r = c->order() - 1;
    mkSamples(knots, params, r);


    // Post refinement
    n = params.size() - 1;
    nPts = n + 1;


    // Visibility
    visibleFaces = {};
    visibleEdges = {0};
    visiblePoints = {0,1};


    // Offsets
    faceOffsets = {};
    lineOffsets = {};
    edgeOffsets = {0.0};
    pointOffsets = {0.0};


    // Indexes
    faceIdxs = {};
    elementIdxs = {};
    edgeIdxs = {0, n};


    // Maps
    faceEdgeMap = {};
    edgePointMap = {{0, {0,1}}};


    // Make data
    mkData();


    // Compute bounding sphere
    computeBoundingSphere();
}


Curve::~Curve()
{
    delete crv;
}


void Curve::mkData()
{
    vertexData.resize(nPts);
    normalData.resize(nPts);

    std::vector<double> points;
    crv->gridEvaluator(points, params);
    for (uint i = 0; i < nPts; i++)
    {
        vertexData[i] = QVector3D(points[3*i], points[3*i+1], points[3*i+2]);
        normalData[i] = QVector3D(0,0,0);
    }

    faceData.resize(0);
    elementData.resize(0);

    edgeData.resize(n);
    for (uint i = 0; i < n; i++)
        edgeData[i] = { i, i+1 };

    pointData.resize(2);
    pointData[0] = 0;
    pointData[1] = n;
}
