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
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "interfaceelem3dtrlin.h"
#include "fei2dtrlin.h"
#include "node.h"
#include "gausspoint.h"
#include "gaussintegrationrule.h"
#include "floatmatrix.h"
#include "floatmatrixf.h"
#include "floatarray.h"
#include "floatarrayf.h"
#include "intarray.h"
#include "mathfem.h"
#include "sm/CrossSections/structuralinterfacecrosssection.h"
#include "classfactory.h"

#ifdef __OOFEG
 #include "oofeggraphiccontext.h"

 #include <Emarkwd3d.h>
#endif

namespace oofem {
REGISTER_Element(InterfaceElement3dTrLin);

FEI2dTrLin InterfaceElement3dTrLin :: interpolation(1, 2);

InterfaceElement3dTrLin :: InterfaceElement3dTrLin(int n, Domain *aDomain) :
    StructuralElement(n, aDomain)
{
    numberOfDofMans = 6;
}


void
InterfaceElement3dTrLin :: computeBmatrixAt(GaussPoint *gp, FloatMatrix &answer, int li, int ui)
//
// Returns linear part of geometrical equations of the receiver at gp.
// Returns the linear part of the B matrix
//
{
    FloatArray n;
    this->interpolation.evalN( n, gp->giveNaturalCoordinates(), FEIElementGeometryWrapper(this) );

    answer.resize(3, 18);
    answer.zero();

    answer.at(1, 10) = answer.at(2, 11) = answer.at(3, 12) = n.at(1);
    answer.at(1, 1)  = answer.at(2, 2)  = answer.at(3, 3)  = -n.at(1);

    answer.at(1, 13) = answer.at(2, 14) = answer.at(3, 15) = n.at(2);
    answer.at(1, 4)  = answer.at(2, 5)  = answer.at(3, 6)  = -n.at(2);

    answer.at(1, 16) = answer.at(2, 17) = answer.at(3, 18) = n.at(3);
    answer.at(1, 7)  = answer.at(2, 8)  = answer.at(3, 9)  = -n.at(3);
}


void
InterfaceElement3dTrLin :: computeGaussPoints()
// Sets up the array of Gauss Points of the receiver.
{
    if ( integrationRulesArray.size() == 0 ) {
        integrationRulesArray.resize( 1 );
        //integrationRulesArray[0] = std::make_unique<LobattoIntegrationRule>(1,domain, 1, 2);
        integrationRulesArray [ 0 ] = std::make_unique<GaussIntegrationRule>(1, this, 1, 3);
        integrationRulesArray [ 0 ]->SetUpPointsOnTriangle(4, _3dInterface);
    }
}


int
InterfaceElement3dTrLin :: computeGlobalCoordinates(FloatArray &answer, const FloatArray &lcoords)
{
    FloatArray n;

    this->interpolation.evalN( n, lcoords, FEIElementGeometryWrapper(this) );

    answer.resize(3);
    answer.zero();
    for ( int i = 1; i <= 3; i++ ) {
        answer.at(1) += n.at(i) * this->giveNode(i)->giveCoordinate(1);
        answer.at(2) += n.at(i) * this->giveNode(i)->giveCoordinate(2);
        answer.at(3) += n.at(i) * this->giveNode(i)->giveCoordinate(3);
    }

    return 1;
}


bool
InterfaceElement3dTrLin :: computeLocalCoordinates(FloatArray &answer, const FloatArray &gcoords)
{
    OOFEM_ERROR("Not implemented");
    //return false;
}


double
InterfaceElement3dTrLin :: computeVolumeAround(GaussPoint *gp)
// Returns the length of the receiver. This method is valid only if 1
// Gauss point is used.
{
    double determinant, weight, thickness, volume;
    // first compute local nodal coordinates in element plane
    std::vector< FloatArray > lncp(3);
    FloatMatrix lcs(3, 3);
    this->computeLCS(lcs);
    for ( int i = 1; i <= 3; i++ ) {
        lncp[ i - 1 ].beProductOf(lcs, this->giveNode(i)->giveCoordinates());
    }

    determinant = fabs( this->interpolation.giveTransformationJacobian( gp->giveNaturalCoordinates(), FEIVertexListGeometryWrapper(lncp, this->giveGeometryType()) ) );
    weight      = gp->giveWeight();
    thickness   = this->giveCrossSection()->give(CS_Thickness, gp);
    volume      = determinant * weight * thickness;

    return volume;
}


void
InterfaceElement3dTrLin :: computeStressVector(FloatArray &answer, const FloatArray &strain, GaussPoint *gp, TimeStep *tStep)
{
    answer = static_cast< StructuralInterfaceCrossSection* >(this->giveCrossSection())->giveEngTraction_3d(strain, gp, tStep);
}


void
InterfaceElement3dTrLin :: computeConstitutiveMatrixAt(FloatMatrix &answer, MatResponseMode rMode, GaussPoint *gp, TimeStep *tStep)
{
    answer = static_cast< StructuralInterfaceCrossSection* >(this->giveCrossSection())->give3dStiffnessMatrix_Eng(rMode, gp, tStep);
}


void
InterfaceElement3dTrLin :: initializeFrom(InputRecord &ir)
{
    StructuralElement :: initializeFrom(ir);
}


void
InterfaceElement3dTrLin :: giveDofManDofIDMask(int inode, IntArray &answer) const
{
    answer = {D_u, D_v, D_w};
}


void
InterfaceElement3dTrLin :: computeLCS(FloatMatrix &answer)
{
    // computes local coordinate system unit vectors (expressed in global cs)
    // unit vectors are stored rowwise
    FloatArray xl(3), yl(3), zl(3), t2(3);

    // compute local x-axis xl (node(2)-node(1))
    xl.at(1) = this->giveNode(2)->giveCoordinate(1) - this->giveNode(1)->giveCoordinate(1);
    xl.at(2) = this->giveNode(2)->giveCoordinate(2) - this->giveNode(1)->giveCoordinate(2);
    xl.at(3) = this->giveNode(2)->giveCoordinate(3) - this->giveNode(1)->giveCoordinate(3);

    xl.normalize();

    // compute another in-plane tangent vector t2 (node(3)-node(1))
    t2.at(1) = this->giveNode(3)->giveCoordinate(1) - this->giveNode(1)->giveCoordinate(1);
    t2.at(2) = this->giveNode(3)->giveCoordinate(2) - this->giveNode(1)->giveCoordinate(2);
    t2.at(3) = this->giveNode(3)->giveCoordinate(3) - this->giveNode(1)->giveCoordinate(3);

    // compute local z axis as product of xl and t2
    zl.beVectorProductOf(xl, t2);
    zl.normalize();

    // compute local y axis as product of zl x xl
    yl.beVectorProductOf(zl, xl);

    answer.resize(3, 3);
    for ( int i = 1; i <= 3; i++ ) {
        answer.at(1, i) = xl.at(i);
        answer.at(2, i) = yl.at(i);
        answer.at(3, i) = zl.at(i);
    }
}


bool
InterfaceElement3dTrLin :: computeGtoLRotationMatrix(FloatMatrix &answer)
{
    // planar geometry is assumed
    FloatMatrix lcs(3, 3);
    this->computeLCS(lcs);

    answer.resize(18, 18);
    for ( int i = 0; i < 6; i++ ) {
        for ( int j = 1; j <= 3; j++ ) {
            answer.at(i * 3 + 1, i * 3 + j) = lcs.at(3, j);
            answer.at(i * 3 + 2, i * 3 + j) = lcs.at(1, j);
            answer.at(i * 3 + 3, i * 3 + j) = lcs.at(2, j);
        }
    }

    return 1;
}


#ifdef __OOFEG
void InterfaceElement3dTrLin :: drawRawGeometry(oofegGraphicContext &gc, TimeStep *tStep)
{
    GraphicObj *go;
    //  if (!go) { // create new one
    WCRec p [ 3 ]; /* triangle */
    if ( !gc.testElementGraphicActivity(this) ) {
        return;
    }

    EASValsSetLineWidth(OOFEG_RAW_GEOMETRY_WIDTH);
    EASValsSetColor( gc.getElementColor() );
    EASValsSetEdgeColor( gc.getElementEdgeColor() );
    EASValsSetEdgeFlag(true);
    EASValsSetLayer(OOFEG_RAW_GEOMETRY_LAYER);
    p [ 0 ].x = ( FPNum ) this->giveNode(1)->giveCoordinate(1);
    p [ 0 ].y = ( FPNum ) this->giveNode(1)->giveCoordinate(2);
    p [ 0 ].z = ( FPNum ) this->giveNode(1)->giveCoordinate(3);
    p [ 1 ].x = ( FPNum ) this->giveNode(2)->giveCoordinate(1);
    p [ 1 ].y = ( FPNum ) this->giveNode(2)->giveCoordinate(2);
    p [ 1 ].z = ( FPNum ) this->giveNode(2)->giveCoordinate(3);
    p [ 2 ].x = ( FPNum ) this->giveNode(3)->giveCoordinate(1);
    p [ 2 ].y = ( FPNum ) this->giveNode(3)->giveCoordinate(2);
    p [ 2 ].z = ( FPNum ) this->giveNode(3)->giveCoordinate(3);

    go =  CreateTriangle3D(p);
    EGWithMaskChangeAttributes(WIDTH_MASK | COLOR_MASK | EDGE_COLOR_MASK | EDGE_FLAG_MASK | LAYER_MASK, go);
    EGAttachObject(go, ( EObjectP ) this);
    EMAddGraphicsToModel(ESIModel(), go);
}


void InterfaceElement3dTrLin :: drawDeformedGeometry(oofegGraphicContext &gc, TimeStep *tStep, UnknownType type)
{ }


void InterfaceElement3dTrLin :: drawScalar(oofegGraphicContext &gc, TimeStep *tStep)
{ }

#endif
} // end namespace oofem
