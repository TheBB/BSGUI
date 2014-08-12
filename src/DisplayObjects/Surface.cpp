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

#include "DisplayObjects/Surface.h"


Surface::Surface(Go::SplineSurface *s)
    : DisplayObject()
    , srf(s)
{
    s->basis(0).knotsSimple(uKnots);
    s->basis(1).knotsSimple(vKnots);


    // Pre refinement
    ntU = uKnots.size() - 1;
    ntV = vKnots.size() - 1;

    ntPtsU = ntU + 1;
    ntPtsV = ntV + 1;
    ntPts = ntPtsU * ntPtsV;

    ntElems = ntU * ntV;


    // Refinement
    rU = s->order_u() + 2;
    rV = s->order_v() + 2;

    mkSamples(uKnots, uParams, rU);
    mkSamples(vKnots, vParams, rV);


    // Post refinement
    nU = (uParams.size() - 1) * (s->rational() ? 5 : 1);
    nV = (vParams.size() - 1) * (s->rational() ? 5 : 1);

    nPtsU = nU + 1;
    nPtsV = nV + 1;
    nPts = nPtsU * nPtsV;

    nElems = nU * nV;
    nElemLines = nU * (ntV-1) + nV * (ntU - 1);


    // Visibility
    visibleFaces  = {0};
    visibleEdges  = {0,1,2,3};
    visiblePoints = {0,1,2,3};


    // Offsets
    faceOffsets = {0.0};
    lineOffsets = {-0.0001, 0.0001};
    edgeOffsets = {0.0};
    pointOffsets = {0.0};


    // Indexes
    faceIdxs    = {0, nU*nV};
    elementIdxs = {0, nElemLines};
    edgeIdxs    = {0, nU, 2*nU, 2*nU + nV, 2*(nU + nV) };


    // Maps
    faceEdgeMap  = {{0, {0,1,2,3}}};
    edgePointMap = {{0, {0,1}},
                    {1, {2,3}},
                    {2, {0,2}},
                    {3, {1,3}}};


    // Make data
    mkVertexData();
    mkFaceData();
    mkElementData();
    mkEdgeData();
    mkPointData();


    // Compute bounding sphere
    computeBoundingSphere();
}


Surface::~Surface()
{
    delete srf;
}


void Surface::mkVertexData()
{
    vertexData.resize(nPts);
    normalData.resize(nPts);

    std::vector<double> points, d1, d2;
    srf->gridEvaluator(uParams, vParams, points, d1, d2);
    for (int i = 0; i < nPtsU; i++)
        for (int j = 0; j < nPtsV; j++)
        {
            uint idx = nPtsU * j + i;
            vertexData[pt(i,j)] = QVector3D(points[3*idx], points[3*idx+1], points[3*idx+2]);

            Go::Point deriv1 = Go::Point(d1[3*idx], d1[3*idx+1], d1[3*idx+2]);
            Go::Point deriv2 = Go::Point(d2[3*idx], d2[3*idx+1], d2[3*idx+2]);
            Go::Point norm = deriv1 % deriv2;

            normalData[pt(i,j)] = QVector3D(norm[0], norm[1], norm[2]).normalized();
        }
}


void Surface::mkFaceData()
{
    faceData.resize(nElems);

    for (int i = 0; i < nU; i++)
        for (int j = 0; j < nV; j++)
            faceData[face(i,j)] = { pt(i,j), pt(i+1,j), pt(i+1,j+1), pt(i,j+1) };
}


void Surface::mkElementData()
{
    elementData.resize(nElemLines);

    for (int i = 0; i < nU; i++)
        for (int j = 1; j < ntV; j++)
            elementData[uElmt(i,j-1)] = { pt(i, rV*j), pt(i+1, rV*j) };

    for (int i = 0; i < nV; i++)
        for (int j = 1; j < ntU; j++)
            elementData[vElmt(i,j-1)] = { pt(rU*j, i), pt(rU*j, i+1) };
}


void Surface::mkEdgeData()
{
    edgeData.resize(2 * (nU + nV));

    for (bool b : {true, false})
    {
        for (int i = 0; i < nU; i++)
            edgeData[uEdge(i,b)] = { pt(i, b ? nV : 0), pt(i+1, b ? nV : 0) };
        for (int i = 0; i < nV; i++)
            edgeData[vEdge(i,b)] = { pt(b ? nU : 0, i), pt(b ? nU : 0, i+1) };
    }
}


void Surface::mkPointData()
{
    pointData.resize(4);
    pointData[0] = pt(0,0);
    pointData[1] = pt(nU,0);
    pointData[2] = pt(0,nV);
    pointData[3] = pt(nU,nV);
}
