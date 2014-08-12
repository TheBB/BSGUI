/*
 * Copyright (C) 2014 SINTEF ICT,
 * Applied Mathematics, Norway.
 *
 * Contact information:
 * E-mail: eivind.fonn@sintef.no
 * SINTEF ICT, Department of Applied Mathematics,
 * P.O. Box 4760 Sluppen,
 * 7045 Trondheim, Norway.
 *
 * This file is part of BSGUI.
 *
 * BSGUI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * BSGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with GoTools. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In accordance with Section 7(b) of the GNU Affero General Public
 * License, a covered work must retain the producer line in every data
 * file that is created or manipulated using GoTools.
 *
 * Other Usage
 * You can be released from the requirements of the license by purchasing
 * a commercial license. Buying such a license is mandatory as soon as you
 * develop commercial activities involving the GoTools library without
 * disclosing the source code of your own applications.
 *
 * This file may be used in accordance with the terms contained in a
 * written agreement between you and SINTEF ICT.
 */

#include "DisplayObjects/Volume.h"


Volume::Volume(Go::SplineVolume *v)
    : DisplayObject()
    , vol(v)
{
    v->basis(0).knotsSimple(uKnots);
    v->basis(1).knotsSimple(vKnots);
    v->basis(2).knotsSimple(wKnots);


    // Pre refinement
    ntU = uKnots.size() - 1;
    ntV = vKnots.size() - 1;
    ntW = wKnots.size() - 1;

    ntPtsU = ntU + 1;
    ntPtsV = ntV + 1;
    ntPtsW = ntW + 1;
    ntPts = 2*ntPtsU*ntPtsV + 2*ntPtsU*(ntPtsW-2) + 2*(ntPtsV-2)*(ntPtsW-2);

    ntElems = 2*ntU*ntV + 2*ntU*ntW + 2*ntV*ntW;


    // Refinement
    rU = (v->order(0) + 2) * (v->rational() ? 5 : 1);
    rV = (v->order(1) + 2) * (v->rational() ? 5 : 1);
    rW = (v->order(2) + 2) * (v->rational() ? 5 : 1);

    mkSamples(uKnots, uParams, rU);
    mkSamples(vKnots, vParams, rV);
    mkSamples(wKnots, wParams, rW);


    // Post refinement
    nU = uParams.size() - 1;
    nV = vParams.size() - 1;
    nW = wParams.size() - 1;

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


    // Offsets
    faceOffsets = {-0.0003};
    lineOffsets = {-0.0002, -0.0004};
    edgeOffsets = {-0.0001};
    pointOffsets = {0};


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
    mkVertexData();
    mkFaceData();
    mkElementData();
    mkEdgeData();
    mkPointData();


    // Compute bounding sphere
    computeBoundingSphere();
}


Volume::~Volume()
{
    delete vol;
}


void Volume::mkVertexData()
{
    vertexData.resize(nPts);
    normalData.resize(nPts);

    std::vector<std::shared_ptr<Go::SplineSurface>> surfaces = vol->getBoundarySurfaces(true);
    std::vector<double> points, d1, d2;

    for (bool b : {true, false})
    {
        surfaces[b ? 5 : 4]->gridEvaluator(uParams, vParams, points, d1, d2);
        for (int i = 0; i < nPtsU; i++)
            for (int j = 0; j < nPtsV; j++)
            {
                uint idx = nPtsU * j + i;
                vertexData[uvPt(i,j,b)] = QVector3D(points[3*idx], points[3*idx+1], points[3*idx+2]);

                Go::Point deriv1 = Go::Point(d1[3*idx], d1[3*idx+1], d1[3*idx+2]);
                Go::Point deriv2 = Go::Point(d2[3*idx], d2[3*idx+1], d2[3*idx+2]);
                Go::Point norm = b ? (deriv1 % deriv2) : (deriv2 % deriv1);

                normalData[uvPt(i,j,b)] += QVector3D(norm[0], norm[1], norm[2]).normalized();
            }

        surfaces[b ? 3 : 2]->gridEvaluator(uParams, wParams, points, d1, d2);
        for (int i = 0; i < nPtsU; i++)
            for (int j = 0; j < nPtsW; j++)
            {
                uint idx = nPtsU * j + i;
                vertexData[uwPt(i,j,b)] = QVector3D(points[3*idx], points[3*idx+1], points[3*idx+2]);

                Go::Point deriv1 = Go::Point(d1[3*idx], d1[3*idx+1], d1[3*idx+2]);
                Go::Point deriv2 = Go::Point(d2[3*idx], d2[3*idx+1], d2[3*idx+2]);
                Go::Point norm = b ? (deriv2 % deriv1) : (deriv1 % deriv2);

                normalData[uwPt(i,j,b)] += QVector3D(norm[0], norm[1], norm[2]).normalized();
            }

        surfaces[b ? 1 : 0]->gridEvaluator(vParams, wParams, points, d1, d2);
        for (int i = 0; i < nPtsV; i++)
            for (int j = 0; j < nPtsW; j++)
            {
                uint idx = nPtsV * j + i;
                vertexData[vwPt(i,j,b)] = QVector3D(points[3*idx], points[3*idx+1], points[3*idx+2]);

                Go::Point deriv1 = Go::Point(d1[3*idx], d1[3*idx+1], d1[3*idx+2]);
                Go::Point deriv2 = Go::Point(d2[3*idx], d2[3*idx+1], d2[3*idx+2]);
                Go::Point norm = b ? (deriv1 % deriv2) : (deriv2 % deriv1);

                normalData[vwPt(i,j,b)] += QVector3D(norm[0], norm[1], norm[2]).normalized();
            }
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
