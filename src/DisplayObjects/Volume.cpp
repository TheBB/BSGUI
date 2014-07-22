#include "DisplayObjects/Volume.h"


Volume::Volume(QVector3D center)
    : DisplayObject(27)
{
    // Pre refinement
    ntU = 3; ntV = 4; ntW = 5;

    ntPtsU = ntU + 1;
    ntPtsV = ntV + 1;
    ntPtsW = ntW + 1;
    ntPts = 2*ntPtsU*ntPtsV + 2*ntPtsU*(ntPtsW-2) + 2*(ntPtsV-2)*(ntPtsW-2);

    ntElems = 2*ntU*ntV + 2*ntU*ntW + 2*ntV*ntW;

    // Refinement
    rU = 2; rV = 2; rW = 2;

    // Post refinement
    nU = rU * ntU;
    nV = rV * ntV;
    nW = rW * ntW;

    nPtsU = nU + 1;
    nPtsV = nV + 1;
    nPtsW = nW + 1;
    nPts = 2*nPtsU*nPtsV + 2*nPtsU*(nPtsW-2) + 2*(nPtsV-2)*(nPtsW-2);

    nElems = 2*nU*nV + 2*nU*nW + 2*nV*nW;

    nLinesUV = ntU*(ntV-1) + ntV*(ntU-1);
    nLinesUW = ntU*(ntW-1) + ntW*(ntU-1);
    nLinesVW = ntV*(ntW-1) + ntW*(ntV-1);

    nElemLines = 2 * (nLinesUV + nLinesUW + nLinesVW);

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
    uint baseUW = 2*nPtsU*nPtsV;
    uint baseVW = 2*nPtsU*nPtsV + 2*nPtsU*(nPtsW-2);

    vertexDataFaces.resize(nPts);

    for (bool b : {true, false})
    {
        for (int i = 0; i < nPtsU; i++)
            for (int j = 0; j < nPtsV; j++)
                vertexDataFaces[uvPt(i,j,b)] = center + QVector3D(lpt(i,nPtsU), lpt(j,nPtsV), b ? 1.0 : -1.0);
        for (int i = 0; i < nPtsU; i++)
            for (int j = 1; j < nPtsW - 1; j++)
                vertexDataFaces[uwPt(i,j,b)] = center + QVector3D(lpt(i,nPtsU), b ? 1.0 : -1.0, lpt(j,nPtsW));
        for (int i = 1; i < nPtsV - 1; i++)
            for (int j = 1; j < nPtsW - 1; j++)
                vertexDataFaces[vwPt(i,j,b)] = center + QVector3D(b ? 1.0 : -1.0, lpt(i,nPtsV), lpt(j,nPtsW));
    }

    vertexDataGrid.resize(ntPts);

    for (bool b : {true, false})
    {
        for (int i = 0; i < ntPtsU; i++)
            for (int j = 0; j < ntPtsV; j++)
                vertexDataGrid[uvtPt(i,j,b)] = center + 1.001 * QVector3D(lpt(i,ntPtsU),
                                                                          lpt(j,ntPtsV),
                                                                          b ? 1.0 : -1.0);
        for (int i = 0; i < ntPtsU; i++)
            for (int j = 1; j < ntPtsW - 1; j++)
                vertexDataGrid[uwtPt(i,j,b)] = center + 1.001 * QVector3D(lpt(i,ntPtsU),
                                                                          b ? 1.0 : -1.0,
                                                                          lpt(j,ntPtsW));
        for (int i = 1; i < ntPtsV - 1; i++)
            for (int j = 1; j < ntPtsW - 1; j++)
                vertexDataGrid[vwtPt(i,j,b)] = center + 1.001 * QVector3D(b ? 1.0 : -1.0,
                                                                          lpt(i,ntPtsV),
                                                                          lpt(j,ntPtsW));
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
        for (int i = 0; i < ntU; i++)
        {
            for (int j = 1; j < ntV; j++)
                elementData[uElmt(i, j-1, a, false)] = { uvtPt(i, j, a), uvtPt(i+1, j, a) };
            for (int j = 1; j < ntW; j++)
                elementData[uElmt(i, j-1, a, true)] = { uwtPt(i, j, a), uwtPt(i+1, j, a) };
        }
        for (int i = 0; i < ntV; i++)
        {
            for (int j = 1; j < ntU; j++)
                elementData[vElmt(i, j-1, a, false)] = { uvtPt(j, i, a), uvtPt(j, i+1, a) };
            for (int j = 1; j < ntW; j++)
                elementData[vElmt(i, j-1, a, true)] = { vwtPt(i, j, a), vwtPt(i+1, j, a) };
        }
        for (int i = 0; i < ntW; i++)
        {
            for (int j = 1; j < ntU; j++)
                elementData[wElmt(i, j-1, a, false)] = { uwtPt(j, i, a), uwtPt(j, i+1, a) };
            for (int j = 1; j < ntV; j++)
                elementData[wElmt(i, j-1, a, true)] = { vwtPt(j, i, a), vwtPt(j, i+1, a) };
        }
    }
}


void Volume::mkEdgeData()
{
    edgeData.resize(4 * (ntU + ntV + ntW));

    for (bool a : {true, false})
        for (bool b : {true, false})
        {
            for (int i = 0; i < ntU; i++)
                edgeData[uEdge(i,a,b)] = { uvtPt(i, a ? ntV : 0, b), uvtPt(i+1, a ? ntV : 0, b) };
            for (int i = 0; i < ntV; i++)
                edgeData[vEdge(i,a,b)] = { uvtPt(a ? ntU : 0, i, b), uvtPt(a ? ntU : 0, i+1, b) };
            for (int i = 0; i < ntW; i++)
                edgeData[wEdge(i,a,b)] = { uwtPt(a ? ntU : 0, i, b), uwtPt(a ? ntU : 0, i+1, b) };
        }
}


void Volume::mkPointData()
{
    pointData.resize(8);

    pointData[0] = uvtPt(0, 0, false);
    pointData[1] = uvtPt(ntU, 0, false);
    pointData[2] = uvtPt(0, ntV, false);
    pointData[3] = uvtPt(ntU, ntV, false);
    pointData[4] = uvtPt(0, 0, true);
    pointData[5] = uvtPt(ntU, 0, true);
    pointData[6] = uvtPt(0, ntV, true);
    pointData[7] = uvtPt(ntU, ntV, true);
}
