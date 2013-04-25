/*
 *
 *                 #####    #####   ######  ######  ###   ###
 *               ##   ##  ##   ##  ##      ##      ## ### ##
 *              ##   ##  ##   ##  ####    ####    ##  #  ##
 *             ##   ##  ##   ##  ##      ##      ##     ##
 *            ##   ##  ##   ##  ##      ##      ##     ##
 *            #####    #####   ##      ######  ##     ##
 *
 *
 *             OOFEM : Object Oriented Finite Element Code
 *
 *               Copyright (C) 1993 - 2013   Borek Patzak
 *
 *
 *
 *       Czech Technical University, Faculty of Civil Engineering,
 *   Department of Structural Mechanics, 166 29 Prague, Czech Republic
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "fei3dhexatriquad.h"
#include "intarray.h"
#include "floatarray.h"
#include "floatmatrix.h"

namespace oofem {
void
FEI3dHexaTriQuad :: evalN(FloatArray &answer, const FloatArray &lcoords, const FEICellGeometry &cellgeo)
{
    double u = lcoords.at(1);
    double v = lcoords.at(2);
    double w = lcoords.at(3);

    double a[] = {0.5*(u-1.0)*u, 0.5*(u+1.0)*u, 1.0-u*u};
    double b[] = {0.5*(v-1.0)*v, 0.5*(v+1.0)*v, 1.0-v*v};
    double c[] = {0.5*(w-1.0)*v, 0.5*(w+1.0)*w, 1.0-w*w};

    answer.resize(27);

    answer.at( 1) = a[0] * b[0] * c[0];
    answer.at(17) = a[1] * b[0] * c[0];
    answer.at( 5) = a[2] * b[0] * c[0];

    answer.at( 9) = a[0] * b[1] * c[0];
    answer.at(23) = a[1] * b[1] * c[0];
    answer.at(13) = a[2] * b[1] * c[0];

    answer.at( 2) = a[0] * b[2] * c[0];
    answer.at(18) = a[1] * b[2] * c[0];
    answer.at( 6) = a[2] * b[2] * c[0];

    answer.at(12) = a[0] * b[0] * c[1];
    answer.at(26) = a[1] * b[0] * c[1];
    answer.at(16) = a[2] * b[0] * c[1];

    answer.at(21) = a[0] * b[1] * c[1];
    answer.at(27) = a[1] * b[1] * c[1];
    answer.at(22) = a[2] * b[1] * c[1];

    answer.at(10) = a[0] * b[2] * c[1];
    answer.at(24) = a[1] * b[2] * c[1];
    answer.at(14) = a[2] * b[2] * c[1];

    answer.at( 4) = a[0] * b[0] * c[2];
    answer.at(20) = a[1] * b[0] * c[2];
    answer.at( 8) = a[2] * b[0] * c[2];

    answer.at(11) = a[0] * b[1] * c[2];
    answer.at(25) = a[1] * b[1] * c[2];
    answer.at(15) = a[2] * b[1] * c[2];

    answer.at( 3) = a[0] * b[2] * c[2];
    answer.at(19) = a[1] * b[2] * c[2];
    answer.at( 7) = a[2] * b[2] * c[2];
}



void
FEI3dHexaTriQuad :: surfaceEvalN(FloatArray &answer, int isurf, const FloatArray &lcoords, const FEICellGeometry &cellgeo)
{
    double u = lcoords.at(1);
    double v = lcoords.at(2);

    double a[] = {0.5*(u-1.)*u, 0.5*(u+1.)*u, 1.-u*u};
    double b[] = {0.5*(v-1.)*v, 0.5*(v+1.)*v, 1.-v*v};

    answer.resize(9);

    answer.at(1) = a[0] * b[0];
    answer.at(5) = a[1] * b[0];
    answer.at(2) = a[2] * b[0];

    answer.at(8) = a[0] * b[1];
    answer.at(9) = a[1] * b[1];
    answer.at(6) = a[2] * b[1];

    answer.at(4) = a[0] * b[2];
    answer.at(7) = a[1] * b[2];
    answer.at(3) = a[2] * b[2];
}


double
FEI3dHexaTriQuad :: surfaceEvalNormal(FloatArray &answer, int isurf, const FloatArray &lcoords, const FEICellGeometry &cellgeo)
{
    IntArray snodes;
    FloatArray e1, e2, dNdu(9), dNdv(9);

    double u = lcoords.at(1);
    double v = lcoords.at(2);

    double a[] = {0.5*(u-1.)*u, 0.5*(u+1.)*u, 1.-u*u};
    double b[] = {0.5*(v-1.)*v, 0.5*(v+1.)*v, 1.-v*v};

    double da[] = {u - 0.5, u + 0.5, -2. * u};
    double db[] = {v - 0.5, v + 0.5, -2. * v};

    dNdu.at(1) = da[0] * b[0];
    dNdu.at(5) = da[1] * b[0];
    dNdu.at(2) = da[2] * b[0];
    dNdu.at(8) = da[0] * b[1];
    dNdu.at(9) = da[1] * b[1];
    dNdu.at(6) = da[2] * b[1];
    dNdu.at(4) = da[0] * b[2];
    dNdu.at(7) = da[1] * b[2];
    dNdu.at(3) = da[2] * b[2];

    dNdv.at(1) = a[0] * db[0];
    dNdv.at(5) = a[1] * db[0];
    dNdv.at(2) = a[2] * db[0];
    dNdv.at(8) = a[0] * db[1];
    dNdv.at(9) = a[1] * db[1];
    dNdv.at(6) = a[2] * db[1];
    dNdv.at(4) = a[0] * db[2];
    dNdv.at(7) = a[1] * db[2];
    dNdv.at(3) = a[2] * db[2];

    this->computeLocalSurfaceMapping(snodes, isurf);
    for ( int i = 1; i <= 9; ++i ) {
        e1.add(dNdu.at(i), *cellgeo.giveVertexCoordinates(snodes.at(i)));
        e2.add(dNdv.at(i), *cellgeo.giveVertexCoordinates(snodes.at(i)));
    }

    answer.beVectorProductOf(e1, e2);
    return answer.normalize();
}


void
FEI3dHexaTriQuad :: computeLocalSurfaceMapping(IntArray &nodes, int isurf)
{
    ///@todo I haven't carefully checked if the order here is completely consistent.
    if ( isurf == 1 ) {
        nodes.setValues(9,  2, 1, 4, 3,  9, 12, 11, 10, 21);
    } else if ( isurf == 2 ) {
        nodes.setValues(9,  5, 6, 7, 8, 13, 14, 15, 16, 22);
    } else if ( isurf == 3 ) {
        nodes.setValues(9,  1, 5, 6, 2, 17, 13, 18,  9, 23);
    } else if ( isurf == 4 ) {
        nodes.setValues(9,  2, 3, 7, 6, 10, 19, 14, 18, 24);
    } else if ( isurf == 5 ) {
        nodes.setValues(9,  3, 4, 8, 7, 11, 20, 15, 19, 25);
    } else if ( isurf == 6 ) {
        nodes.setValues(9,  4, 1, 5, 8, 12, 17, 16, 20, 26);
    } else {
        OOFEM_ERROR2("FEI3dHexaTriQuad :: computeLocalSurfaceMapping: wrong surface number (%d)", isurf);
    }
}


void
FEI3dHexaTriQuad :: giveLocalDerivative(FloatMatrix &dN, const FloatArray &lcoords)
{
    double u, v, w;
    u = lcoords.at(1);
    v = lcoords.at(2);
    w = lcoords.at(3);

    // Helpers expressions;
    double a[] = {0.5*(u-1.0)*u, 0.5*(u+1.0)*u, 1.0-u*u};
    double b[] = {0.5*(v-1.0)*v, 0.5*(v+1.0)*v, 1.0-v*v};
    double c[] = {0.5*(w-1.0)*v, 0.5*(w+1.0)*w, 1.0-w*w};

    double da[] = {-0.5 + u, 0.5 + u, -2.0 * u};
    double db[] = {-0.5 + v, 0.5 + v, -2.0 * v};
    double dc[] = {-0.5 + w, 0.5 + w, -2.0 * w};

    dN.resize(27, 3);

    dN.at( 1, 1) = da[0] * b[0] * c[0];
    dN.at(17, 1) = da[1] * b[0] * c[0];
    dN.at( 5, 1) = da[2] * b[0] * c[0];

    dN.at( 9, 1) = da[0] * b[1] * c[0];
    dN.at(23, 1) = da[1] * b[1] * c[0];
    dN.at(13, 1) = da[2] * b[1] * c[0];

    dN.at( 2, 1) = da[0] * b[2] * c[0];
    dN.at(18, 1) = da[1] * b[2] * c[0];
    dN.at( 6, 1) = da[2] * b[2] * c[0];

    dN.at(12, 1) = da[0] * b[0] * c[1];
    dN.at(26, 1) = da[1] * b[0] * c[1];
    dN.at(16, 1) = da[2] * b[0] * c[1];

    dN.at(21, 1) = da[0] * b[1] * c[1];
    dN.at(27, 1) = da[1] * b[1] * c[1];
    dN.at(22, 1) = da[2] * b[1] * c[1];

    dN.at(10, 1) = da[0] * b[2] * c[1];
    dN.at(24, 1) = da[1] * b[2] * c[1];
    dN.at(14, 1) = da[2] * b[2] * c[1];

    dN.at( 4, 1) = da[0] * b[0] * c[2];
    dN.at(20, 1) = da[1] * b[0] * c[2];
    dN.at( 8, 1) = da[2] * b[0] * c[2];

    dN.at(11, 1) = da[0] * b[1] * c[2];
    dN.at(25, 1) = da[1] * b[1] * c[2];
    dN.at(15, 1) = da[2] * b[1] * c[2];

    dN.at( 3, 1) = da[0] * b[2] * c[2];
    dN.at(19, 1) = da[1] * b[2] * c[2];
    dN.at( 7, 1) = da[2] * b[2] * c[2];

    //

    dN.at( 1, 2) = a[0] * db[0] * c[0];
    dN.at(17, 2) = a[1] * db[0] * c[0];
    dN.at( 5, 2) = a[2] * db[0] * c[0];

    dN.at( 9, 2) = a[0] * db[1] * c[0];
    dN.at(23, 2) = a[1] * db[1] * c[0];
    dN.at(13, 2) = a[2] * db[1] * c[0];

    dN.at( 2, 2) = a[0] * db[2] * c[0];
    dN.at(18, 2) = a[1] * db[2] * c[0];
    dN.at( 6, 2) = a[2] * db[2] * c[0];

    dN.at(12, 2) = a[0] * db[0] * c[1];
    dN.at(26, 2) = a[1] * db[0] * c[1];
    dN.at(16, 2) = a[2] * db[0] * c[1];

    dN.at(21, 2) = a[0] * db[1] * c[1];
    dN.at(27, 2) = a[1] * db[1] * c[1];
    dN.at(22, 2) = a[2] * db[1] * c[1];

    dN.at(10, 2) = a[0] * db[2] * c[1];
    dN.at(24, 2) = a[1] * db[2] * c[1];
    dN.at(14, 2) = a[2] * db[2] * c[1];

    dN.at( 4, 2) = a[0] * db[0] * c[2];
    dN.at(20, 2) = a[1] * db[0] * c[2];
    dN.at( 8, 2) = a[2] * db[0] * c[2];

    dN.at(11, 2) = a[0] * db[1] * c[2];
    dN.at(25, 2) = a[1] * db[1] * c[2];
    dN.at(15, 2) = a[2] * db[1] * c[2];

    dN.at( 3, 2) = a[0] * db[2] * c[2];
    dN.at(19, 2) = a[1] * db[2] * c[2];
    dN.at( 7, 2) = a[2] * db[2] * c[2];

    //

    dN.at( 1, 3) = a[0] * b[0] * dc[0];
    dN.at(17, 3) = a[1] * b[0] * dc[0];
    dN.at( 5, 3) = a[2] * b[0] * dc[0];

    dN.at( 9, 3) = a[0] * b[1] * dc[0];
    dN.at(23, 3) = a[1] * b[1] * dc[0];
    dN.at(13, 3) = a[2] * b[1] * dc[0];

    dN.at( 2, 3) = a[0] * b[2] * dc[0];
    dN.at(18, 3) = a[1] * b[2] * dc[0];
    dN.at( 6, 3) = a[2] * b[2] * dc[0];

    dN.at(12, 3) = a[0] * b[0] * dc[1];
    dN.at(26, 3) = a[1] * b[0] * dc[1];
    dN.at(16, 3) = a[2] * b[0] * dc[1];

    dN.at(21, 3) = a[0] * b[1] * dc[1];
    dN.at(27, 3) = a[1] * b[1] * dc[1];
    dN.at(22, 3) = a[2] * b[1] * dc[1];

    dN.at(10, 3) = a[0] * b[2] * dc[1];
    dN.at(24, 3) = a[1] * b[2] * dc[1];
    dN.at(14, 3) = a[2] * b[2] * dc[1];

    dN.at( 4, 3) = a[0] * b[0] * dc[2];
    dN.at(20, 3) = a[1] * b[0] * dc[2];
    dN.at( 8, 3) = a[2] * b[0] * dc[2];

    dN.at(11, 3) = a[0] * b[1] * dc[2];
    dN.at(25, 3) = a[1] * b[1] * dc[2];
    dN.at(15, 3) = a[2] * b[1] * dc[2];

    dN.at( 3, 3) = a[0] * b[2] * dc[2];
    dN.at(19, 3) = a[1] * b[2] * dc[2];
    dN.at( 7, 3) = a[2] * b[2] * dc[2];
}

} // end namespace oofem