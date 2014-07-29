#include "DisplayObjects/Volume.h"


Volume::Volume(QVector3D center)
    : DisplayObject(12)
{
    // Pre refinement
    ntU = 3; ntV = 4; ntW = 5;

    ntPtsU = ntU + 1;
    ntPtsV = ntV + 1;
    ntPtsW = ntW + 1;
    ntPts = 2*ntPtsU*ntPtsV + 2*ntPtsU*(ntPtsW-2) + 2*(ntPtsV-2)*(ntPtsW-2);

    ntElems = 2*ntU*ntV + 2*ntU*ntW + 2*ntV*ntW;

    // Refinement
    rU = 1; rV = 1; rW = 1;

    // Post refinement
    nU = rU * ntU;
    nV = rV * ntV;
    nW = rW * ntW;

    nPtsU = nU + 1;
    nPtsV = nV + 1;
    nPtsW = nW + 1;
    nPts = 2*nPtsU*nPtsV + 2*nPtsU*(nPtsW-2) + 2*(nPtsV-2)*(nPtsW-2);

    nElems = 2*nU*nV + 2*nU*nW + 2*nV*nW;

    nLinesUV = nU*(ntV-1) + nV*(ntU-1);
    nLinesUW = nU*(ntW-1) + nW*(ntU-1);
    nLinesVW = nV*(ntW-1) + nW*(ntV-1);

    nElemLines = 2 * (nLinesUV + nLinesUW + nLinesVW);

    // Visibility
    visibleFaces  = {0,1,2,3,4,5};
    visibleEdges  = {0,1,2,3,4,5,6,7,8,9,10,11};
    visiblePoints = {0,1,2,3,4,5,6,7};

    // Indexes
    faceIdxs    = {0, nU*nV, nU*nV*2, 2*nU*nV + nU*nW, 2*(nU*nV + nU*nW), 2*(nU*nV + nU*nW) + nV*nW,
                   2*(nU*nV + nU*nW + nV*nW)};
    elementIdxs = {0, nLinesUV, 2*nLinesUV, 2*nLinesUV + nLinesUW, 2*(nLinesUV + nLinesUW),
                   2*(nLinesUV + nLinesUW) + nLinesVW, 2*(nLinesUV + nLinesUW + nLinesVW)};
    edgeIdxs    = {0, nU, 2*nU, 3*nU, 4*nU, 4*nU + nV, 4*nU + 2*nV, 4*nU + 3*nV, 4*(nU + nV),
                   4*(nU + nV) + nW, 4*(nU + nV) + 2*nW, 4*(nU + nV) + 3*nW, 4*(nU + nV + nW)};

    // Maps
    faceEdgeMap  = {{0, {0,1,4,5}},
                    {1, {2,3,6,7}},
                    {2, {0,2,8,9}},
                    {3, {1,3,10,11}},
                    {4, {4,6,8,10}},
                    {5, {5,7,9,11}}};
    edgePointMap = {{0, {0,1}},
                    {1, {2,3}},
                    {2, {4,5}},
                    {3, {6,7}},
                    {4, {0,2}},
                    {5, {1,3}},
                    {6, {4,6}},
                    {7, {5,7}},
                    {8, {0,4}},
                    {9, {1,5}},
                    {10, {2,6}},
                    {11, {3,7}}};

    // Make data
    mkVertexData(center);
    mkFaceData();
    mkElementData();
    mkEdgeData();
    mkPointData();

    // Compute bounding sphere
    computeBoundingSphere();
}


static inline float lpt(uint i, uint N)
{
    return (float) i/(N-1) * 2.0 - 1.0;
}


void Volume::mkVertexData(QVector3D center)
{
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

    normalData.resize(nPts);

    for (bool b : {true, false})
    {
        for (int i = 1; i < nPtsU - 1; i++)
            for (int j = 1; j < nPtsV - 1; j++)
                normalData[uvPt(i,j,b)] = QVector3D(0.0, 0.0, b ? 1.0 : -1.0);
        for (int i = 1; i < nPtsU - 1; i++)
            for (int j = 1; j < nPtsW - 1; j++)
                normalData[uwPt(i,j,b)] = QVector3D(0.0, b ? 1.0 : -1.0, 0.0);
        for (int i = 1; i < nPtsV - 1; i++)
            for (int j = 1; j < nPtsW - 1; j++)
                normalData[vwPt(i,j,b)] = QVector3D(b ? 1.0 : -1.0, 0.0, 0.0);

        for (int i = 1; i < nPtsU - 1; i++)
        {
            normalData[uvPt(i,0,false)] = QVector3D(0.0, -1.0, -1.0);
            normalData[uvPt(i,nPtsV-1,false)] = QVector3D(0.0, 1.0, -1.0);
            normalData[uvPt(i,0,true)] = QVector3D(0.0, -1.0, 1.0);
            normalData[uvPt(i,nPtsV-1,true)] = QVector3D(0.0, 1.0, 1.0);
        }
        for (int i = 1; i < nPtsV - 1; i++)
        {
            normalData[uvPt(0,i,false)] = QVector3D(-1.0, 0.0, -1.0);
            normalData[uvPt(nPtsU-1,i,false)] = QVector3D(1.0, 0.0, -1.0);
            normalData[uvPt(0,i,true)] = QVector3D(-1.0, 0.0, 1.0);
            normalData[uvPt(nPtsU-1,i,true)] = QVector3D(1.0, 0.0, 1.0);
        }
        for (int i = 1; i < nPtsW - 1; i++)
        {
            normalData[uwPt(0,i,false)] = QVector3D(-1.0, -1.0, 0.0);
            normalData[uwPt(nPtsU-1,i,false)] = QVector3D(1.0, -1.0, 0.0);
            normalData[uwPt(0,i,true)] = QVector3D(-1.0, 1.0, 0.0);
            normalData[uwPt(nPtsU-1,i,true)] = QVector3D(1.0, 1.0, 0.0);
        }

        normalData[uvPt(0,0,false)] = QVector3D(-1.0, -1.0, -1.0);
        normalData[uvPt(nPtsU-1,0,false)] = QVector3D(1.0, -1.0, -1.0);
        normalData[uvPt(0,nPtsV-1,false)] = QVector3D(-1.0, 1.0, -1.0);
        normalData[uvPt(nPtsU-1,nPtsV-1,false)] = QVector3D(1.0, 1.0, -1.0);
        normalData[uvPt(0,0,true)] = QVector3D(-1.0, -1.0, 1.0);
        normalData[uvPt(nPtsU-1,0,true)] = QVector3D(1.0, -1.0, 1.0);
        normalData[uvPt(0,nPtsV-1,true)] = QVector3D(-1.0, 1.0, 1.0);
        normalData[uvPt(nPtsU-1,nPtsV-1,true)] = QVector3D(1.0, 1.0, 1.0);
    }
}


void Volume::mkFaceData()
{
    faceData.resize(nElems);

    for (bool b : {true, false})
    {
        for (int i = 0; i < nU; i++)
            for (int j = 0; j < nV; j++)
                faceData[uvFace(i,j,b)] = { uvPt(i,j,b), uvPt(i+1,j,b), uvPt(i+1,j+1,b), uvPt(i,j+1,b) };
        for (int i = 0; i < nU; i++)
            for (int j = 0; j < nW; j++)
                faceData[uwFace(i,j,b)] = { uwPt(i,j,b), uwPt(i+1,j,b), uwPt(i+1,j+1,b), uwPt(i,j+1,b) };
        for (int i = 0; i < nV; i++)
            for (int j = 0; j < nW; j++)
                faceData[vwFace(i,j,b)] = { vwPt(i,j,b), vwPt(i+1,j,b), vwPt(i+1,j+1,b), vwPt(i,j+1,b) };
    }
}


void Volume::mkElementData()
{
    elementData.resize(nElemLines);

    for (bool a : {false, true})
    {
        for (int i = 0; i < nU; i++)
        {
            for (int j = 1; j < ntV; j++)
                elementData[uElmt(i, j-1, a, false)] = { uvPt(i, rV*j, a), uvPt(i+1, rV*j, a) };
            for (int j = 1; j < ntW; j++)
                elementData[uElmt(i, j-1, a, true)] = { uwPt(i, rW*j, a), uwPt(i+1, rW*j, a) };
        }
        for (int i = 0; i < nV; i++)
        {
            for (int j = 1; j < ntU; j++)
                elementData[vElmt(i, j-1, a, false)] = { uvPt(rU*j, i, a), uvPt(rU*j, i+1, a) };
            for (int j = 1; j < ntW; j++)
                elementData[vElmt(i, j-1, a, true)] = { vwPt(i, rW*j, a), vwPt(i+1, rW*j, a) };
        }
        for (int i = 0; i < nW; i++)
        {
            for (int j = 1; j < ntU; j++)
                elementData[wElmt(i, j-1, a, false)] = { uwPt(rU*j, i, a), uwPt(rU*j, i+1, a) };
            for (int j = 1; j < ntV; j++)
                elementData[wElmt(i, j-1, a, true)] = { vwPt(rV*j, i, a), vwPt(rV*j, i+1, a) };
        }
    }
}


void Volume::mkEdgeData()
{
    edgeData.resize(4 * (nU + nV + nW));

    for (bool a : {true, false})
        for (bool b : {true, false})
        {
            for (int i = 0; i < nU; i++)
                edgeData[uEdge(i,a,b)] = { uvPt(i, a ? nV : 0, b), uvPt(i+1, a ? nV : 0, b) };
            for (int i = 0; i < nV; i++)
                edgeData[vEdge(i,a,b)] = { uvPt(a ? nU : 0, i, b), uvPt(a ? nU : 0, i+1, b) };
            for (int i = 0; i < nW; i++)
                edgeData[wEdge(i,a,b)] = { uwPt(a ? nU : 0, i, b), uwPt(a ? nU : 0, i+1, b) };
        }
}


void Volume::mkPointData()
{
    pointData.resize(8);

    pointData[0] = uvPt(0, 0, false);
    pointData[1] = uvPt(nU, 0, false);
    pointData[2] = uvPt(0, nV, false);
    pointData[3] = uvPt(nU, nV, false);
    pointData[4] = uvPt(0, 0, true);
    pointData[5] = uvPt(nU, 0, true);
    pointData[6] = uvPt(0, nV, true);
    pointData[7] = uvPt(nU, nV, true);
}
