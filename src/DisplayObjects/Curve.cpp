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
    r = (c->order() - 1) * (c->rational() ? 5 : 1);
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
