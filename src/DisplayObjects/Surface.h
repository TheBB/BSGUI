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
