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

#include "sm/Elements/Interfaces/cohsur3d.h"
#include "element.h"
#include "dof.h"
#include "node.h"
#include "particle.h"
#include "gaussintegrationrule.h"
#include "floatmatrix.h"
#include "floatmatrixf.h"
#include "intarray.h"
#include "floatarray.h"
#include "floatarrayf.h"
#include "mathfem.h"
#include "sm/CrossSections/structuralinterfacecrosssection.h"
#include "classfactory.h"

#ifdef __OOFEG
 #include "oofeggraphiccontext.h"
 #include "connectivitytable.h"
 #include "oofegutils.h"
#endif

namespace oofem {
REGISTER_Element(CohesiveSurface3d);

CohesiveSurface3d :: CohesiveSurface3d(int n, Domain *aDomain) : StructuralElement(n, aDomain)
    // Constructor.
{
    numberOfDofMans = -1;
    area   = -1.;
    length = -1.;
    kx = ky = kz = 0;
    kxa = kyb = kzc = 0;
}

void
CohesiveSurface3d :: computeBmatrixAt(GaussPoint *gp, FloatMatrix &answer, int li, int ui)
// Returns the strain-displacement matrix of the receiver.
{
    double x01, y01, z01, x02, y02, z02;
    FloatMatrix Bloc(3, 12);

    Node *nodeA, *nodeB;
    nodeA   = this->giveNode(1);
    nodeB   = this->giveNode(2);

    switch ( numberOfDofMans ) {
    case 2:
        // Coordinate differences
        x01 = nodeA->giveCoordinate(1) - center.at(1);
        y01 = nodeA->giveCoordinate(2) - center.at(2);
        z01 = nodeA->giveCoordinate(3) - center.at(3);
        x02 = nodeB->giveCoordinate(1) - center.at(1);
        y02 = nodeB->giveCoordinate(2) - center.at(2);
        z02 = nodeB->giveCoordinate(3) - center.at(3);

        // B matrix in local coordinates (and without the term 1/length)

        Bloc.zero();

        Bloc.at(1, 1) =  -1.;
        Bloc.at(2, 2) =  -1.;
        Bloc.at(3, 3) =  -1.;

        Bloc.at(1, 5) =   z01;
        Bloc.at(1, 6) =  -y01;
        Bloc.at(2, 4) =  -z01;
        Bloc.at(2, 6) =   x01;
        Bloc.at(3, 4) =   y01;
        Bloc.at(3, 5) =  -x01;

        Bloc.at(1, 7) =   1.;
        Bloc.at(2, 8) =   1.;
        Bloc.at(3, 9) =   1.;

        Bloc.at(1, 11) =  -z02;
        Bloc.at(1, 12) =   y02;
        Bloc.at(2, 10) =   z02;
        Bloc.at(2, 12) =  -x02;
        Bloc.at(3, 10) =  -y02;
        Bloc.at(3, 11) =   x02;

        // Transformation to global coordinates

        answer.resize(3, 12);
        answer.beProductOf(lcs, Bloc);

        // Division by the length

        answer.times(1. / length);

        return;

        break;

    case 3:
        // Coordinate differences
        x01 = nodeA->giveCoordinate(1) - center.at(1);
        y01 = nodeA->giveCoordinate(2) - center.at(2);
        z01 = nodeA->giveCoordinate(3) - center.at(3);
        x02 = nodeB->giveCoordinate(1) + kxa - center.at(1);
        y02 = nodeB->giveCoordinate(2) + kyb - center.at(2);
        z02 = nodeB->giveCoordinate(3) + kzc - center.at(3);

        // B matrix in local coordinates (and without the term 1/length)

        Bloc.zero();

        Bloc.at(1, 1) =  -1.;
        Bloc.at(2, 2) =  -1.;
        Bloc.at(3, 3) =  -1.;

        Bloc.at(1, 5) =   z01;
        Bloc.at(1, 6) =  -y01;
        Bloc.at(2, 4) =  -z01;
        Bloc.at(2, 6) =   x01;
        Bloc.at(3, 4) =   y01;
        Bloc.at(3, 5) =  -x01;

        Bloc.at(1, 7) =   1.;
        Bloc.at(2, 8) =   1.;
        Bloc.at(3, 9) =   1.;

        Bloc.at(1, 11) =  -z02;
        Bloc.at(1, 12) =   y02;
        Bloc.at(2, 10) =   z02;
        Bloc.at(2, 12) =  -x02;
        Bloc.at(3, 10) =  -y02;
        Bloc.at(3, 11) =   x02;

        // Transformation to global coordinates

        FloatMatrix answer2(3, 12);
        answer2.zero();
        answer2.beProductOf(lcs, Bloc);

        // Division by the length

        answer2.times(1. / length);

        // periodic transformation matrix T
        FloatMatrix Tper(12, 18);

        Tper.zero();

        Tper.at(1, 1)     = 1.;
        Tper.at(2, 2)     = 1.;
        Tper.at(3, 3)     = 1.;
        Tper.at(4, 4)     = 1.;
        Tper.at(5, 5)     = 1.;
        Tper.at(6, 6)     = 1.;
        Tper.at(7, 7)     = 1.;
        Tper.at(8, 8)     = 1.;
        Tper.at(9, 9)     = 1.;
        Tper.at(10, 10) = 1.;
        Tper.at(11, 11) = 1.;
        Tper.at(12, 12) = 1.;

        Tper.at(7, 13) = kxa;
        Tper.at(8, 14) = kyb;
        Tper.at(9, 15) = kzc;
        Tper.at(7, 16) = kyb;
        Tper.at(8, 17) = kzc;
        Tper.at(9, 18) = kxa;

        // periodic transformation of Bmatrix
        answer.beProductOf(answer2, Tper);

        return;

        break;
    }
}

void CohesiveSurface3d :: computeGaussPoints()
// Sets up the array of Gauss points of the receiver.
{
    // The Gauss point is used only when methods from crosssection and/or material
    // classes are requested.
    integrationRulesArray.resize( 1 );
    integrationRulesArray [ 0 ] = std::make_unique<GaussIntegrationRule>(1, this);
    this->giveCrossSection()->setupIntegrationPoints(* integrationRulesArray [ 0 ], 1, this);
}


double
CohesiveSurface3d :: computeVolumeAround(GaussPoint *gp)
{
    return area * length;
}


void
CohesiveSurface3d :: giveDofManDofIDMask(int inode, IntArray &answer) const
{
    answer = {D_u, D_v, D_w, R_u, R_v, R_w};
}


void
CohesiveSurface3d :: computeStressVector(FloatArray &answer, const FloatArray &strain, GaussPoint *gp, TimeStep *tStep)
{
    answer = static_cast< StructuralInterfaceCrossSection* >(this->giveCrossSection())->giveEngTraction_3d(strain, gp, tStep);
}


void
CohesiveSurface3d :: computeConstitutiveMatrixAt(FloatMatrix &answer, MatResponseMode rMode, GaussPoint *gp, TimeStep *tStep)
{
    answer = static_cast< StructuralInterfaceCrossSection* >(this->giveCrossSection())->give3dStiffnessMatrix_Eng(rMode, gp, tStep);
}


double
CohesiveSurface3d :: giveLength()
// Returns the length of the receiver.
{
    double dx, dy, dz;
    Node *nodeA, *nodeB;
    nodeA   = this->giveNode(1);
    nodeB   = this->giveNode(2);

    switch ( numberOfDofMans ) {
    case 2:
        if ( length <= 0. ) {
            dx      = nodeB->giveCoordinate(1) - nodeA->giveCoordinate(1);
            dy      = nodeB->giveCoordinate(2) - nodeA->giveCoordinate(2);
            dz      = nodeB->giveCoordinate(3) - nodeA->giveCoordinate(3);
            length  = sqrt(dx * dx + dy * dy + dz * dz);
        }

        return length;

        break;

    case 3:
        if ( length <= 0. ) {
            dx      = nodeB->giveCoordinate(1) + kxa - nodeA->giveCoordinate(1);
            dy      = nodeB->giveCoordinate(2) + kyb - nodeA->giveCoordinate(2);
            dz      = nodeB->giveCoordinate(3) + kzc - nodeA->giveCoordinate(3);
            length  = sqrt(dx * dx + dy * dy + dz * dz);
        }

        return length;

        break;
    }

    return 0.;
}

void
CohesiveSurface3d :: evaluateCenter()
{
    Particle *nodeA, *nodeB;
    double RA, RB, L, aux;

    nodeA = static_cast< Particle * >( this->giveNode(1) );
    nodeB = static_cast< Particle * >( this->giveNode(2) );
    RA = nodeA->giveRadius();
    RB = nodeB->giveRadius();
    L  = giveLength();
    aux = 0.5 + ( RA - RB ) / ( 2. * L );

    switch ( numberOfDofMans ) {
    case 2:
        center.resize(3);
        for ( int i = 1; i <= 3; i++ ) {
            center.at(i) = aux * ( nodeB->giveCoordinate(i) ) + ( 1. - aux ) * ( nodeA->giveCoordinate(i) );
        }

        break;

    case 3:
        center.resize(3);
        center.at(1) = aux * ( nodeB->giveCoordinate(1) + kxa ) + ( 1. - aux ) * ( nodeA->giveCoordinate(1) );
        center.at(2) = aux * ( nodeB->giveCoordinate(2) + kyb ) + ( 1. - aux ) * ( nodeA->giveCoordinate(2) );
        center.at(3) = aux * ( nodeB->giveCoordinate(3) + kzc ) + ( 1. - aux ) * ( nodeA->giveCoordinate(3) );

        break;
    }
}

void
CohesiveSurface3d :: evaluateLocalCoordinateSystem()
//
// Computes unit vectors of local coordinate system, stored by rows.
//
{
    FloatArray lx(3), ly(3), lz(3);

    Node *nodeA, *nodeB;
    nodeA  = this->giveNode(1);
    nodeB  = this->giveNode(2);

    switch ( numberOfDofMans ) {
    case 2:
        lx.at(1) = nodeB->giveCoordinate(1) - nodeA->giveCoordinate(1);
        lx.at(2) = nodeB->giveCoordinate(2) - nodeA->giveCoordinate(2);
        lx.at(3) = nodeB->giveCoordinate(3) - nodeA->giveCoordinate(3);
        lx.normalize();
        break;

    case 3:
        lx.at(1) = nodeB->giveCoordinate(1) + kxa - nodeA->giveCoordinate(1);
        lx.at(2) = nodeB->giveCoordinate(2) + kyb - nodeA->giveCoordinate(2);
        lx.at(3) = nodeB->giveCoordinate(3) + kzc - nodeA->giveCoordinate(3);
        lx.normalize();
        break;
    }

    ly.zero();
    if ( fabs( lx.at(1) ) > fabs( lx.at(2) ) ) {
        ly.at(2) = 1.;
    } else {
        ly.at(1) = 1.;
    }

    lz.beVectorProductOf(lx, ly);
    lz.normalize();
    ly.beVectorProductOf(lz, lx);
    ly.normalize();

    lcs.resize(3, 3);
    for ( int i = 1; i <= 3; i++ ) {
        lcs.at(1, i) = lx.at(i);
        lcs.at(2, i) = ly.at(i);
        lcs.at(3, i) = lz.at(i);
    }
}


void
CohesiveSurface3d :: initializeFrom(InputRecord &ir)
{
    // first call parent
    StructuralElement :: initializeFrom(ir);

    // read the area from the input file
    IR_GIVE_FIELD(ir, area, _IFT_CohSur3d_area);
    if ( area < 0. ) {
        throw ValueInputException(ir, _IFT_CohSur3d_area, "negative area specified");
    }

    // read shift constants of second (periodic) particle form the input file (if defined)
    ///@todo Why not a vector input instead?
    IR_GIVE_OPTIONAL_FIELD(ir, kx, _IFT_CohSur3d_kx);
    IR_GIVE_OPTIONAL_FIELD(ir, ky, _IFT_CohSur3d_ky);
    IR_GIVE_OPTIONAL_FIELD(ir, kz, _IFT_CohSur3d_kz);

    // evaluate number of Dof Managers
    numberOfDofMans = dofManArray.giveSize();
    if ( numberOfDofMans <= 0 ) {
        throw ValueInputException(ir, _IFT_Element_nodes, "unread nodes" );
    }

    if ( ( numberOfDofMans == 3 ) & ( kx == 0 ) & ( ky == 0 ) & ( kz == 0 ) ) {
        throw ValueInputException(ir, _IFT_CohSur3d_kx, "no periodic shift defined" );
    }


    // shifts of periodic particles
    if ( numberOfDofMans == 3 ) {
        Node *nodeC;
        nodeC  = this->giveNode(3);
        kxa = this->kx * nodeC->giveCoordinate(1);
        kyb = this->ky * nodeC->giveCoordinate(2);
        kzc = this->kz * nodeC->giveCoordinate(3);
    }

    // evaluate the length
    giveLength();
    if ( length <= 0. ) {
        throw ValueInputException(ir, _IFT_Element_nodes, "negative length evaluated");
        // evaluate the coordinates of the center
        //evaluateCenter(); /// @todo This will never execute. Verify this / Mikael
    }

    // evaluate the local coordinate system
    evaluateLocalCoordinateSystem();
}


int
CohesiveSurface3d :: computeGlobalCoordinates(FloatArray &answer, const FloatArray &lcoords)
{
    answer = center;
    return 1;
}


#ifdef __OOFEG
void CohesiveSurface3d :: drawRawGeometry(oofegGraphicContext &gc, TimeStep *tStep)
{
    if ( !gc.testElementGraphicActivity(this) ) {
        return;
    }

    //WCRec p[4];
    GraphicObj *go;

    Particle *nodeA = ( Particle * ) this->giveNode(1);
    Particle *nodeB = ( Particle * ) this->giveNode(2);
    //double rA = nodeA -> giveRadius();
    //double rB = nodeB -> giveRadius();
    //double r = (rA+rB)/4.;

    EASValsSetLineWidth(OOFEG_RAW_GEOMETRY_WIDTH);
    EASValsSetColor( gc.getElementColor() );
    EASValsSetEdgeColor( gc.getElementEdgeColor() );
    EASValsSetEdgeFlag(true);
    EASValsSetLayer(OOFEG_RAW_GEOMETRY_LAYER);

    WCRec pl [ 2 ];
    // determine coordinates of the particles connected by this element
    pl [ 0 ].x = ( FPNum ) nodeA->giveCoordinate(1);
    pl [ 0 ].y = ( FPNum ) nodeA->giveCoordinate(2);
    pl [ 0 ].z = ( FPNum ) nodeA->giveCoordinate(3);
    pl [ 1 ].x = ( FPNum ) nodeB->giveCoordinate(1);
    pl [ 1 ].y = ( FPNum ) nodeB->giveCoordinate(2);
    pl [ 1 ].z = ( FPNum ) nodeB->giveCoordinate(3);
    if ( giveNumberOfNodes() == 3 ) {
        // the second particle should be shifted (periodic arrangement)
        Particle *nodeC = ( Particle * ) this->giveNode(3);
        pl [ 1 ].x += kx * ( nodeC->giveCoordinate(1) );
        pl [ 1 ].y += ky * ( nodeC->giveCoordinate(2) );
        pl [ 1 ].z += kz * ( nodeC->giveCoordinate(3) );
    }

    //  plot a line segment connecting the particles
    go = CreateLine3D(pl);
    EGWithMaskChangeAttributes(WIDTH_MASK | COLOR_MASK | LAYER_MASK, go);
    EGAttachObject(go, ( EObjectP ) this);
    EMAddGraphicsToModel(ESIModel(), go);
}

void CohesiveSurface3d :: drawDeformedGeometry(oofegGraphicContext &gc, TimeStep *tStep, UnknownType type)
{
    GraphicObj *go1, *go2;

    if ( !gc.testElementGraphicActivity(this) ) {
        return;
    }

    double defScale = gc.getDefScale();
    WCRec p [ 2 ];
    EASValsSetLineWidth(OOFEG_DEFORMED_GEOMETRY_WIDTH);
    EASValsSetColor( gc.getDeformedElementColor() );
    EASValsSetLayer(OOFEG_DEFORMED_GEOMETRY_LAYER);

    //  get the displaced particle coordinates
    Particle *nodeA = ( Particle * ) giveNode(1);
    Particle *nodeB = ( Particle * ) giveNode(2);
    p [ 0 ].x = nodeA->giveUpdatedCoordinate(1, tStep, defScale);
    p [ 0 ].y = nodeA->giveUpdatedCoordinate(2, tStep, defScale);
    p [ 0 ].z = nodeA->giveUpdatedCoordinate(3, tStep, defScale);

    p [ 1 ].x = nodeB->giveUpdatedCoordinate(1, tStep, defScale);
    p [ 1 ].y = nodeB->giveUpdatedCoordinate(2, tStep, defScale);
    p [ 1 ].z = nodeB->giveUpdatedCoordinate(3, tStep, defScale);

    // plot the displaced particles
    EASValsSetMType(FILLED_CIRCLE_MARKER);
    EASValsSetColor( gc.getNodeColor() );
    EASValsSetMSize(6);

    // plot the first particle
    go1 = CreateMarker3D(p);
    EGWithMaskChangeAttributes(COLOR_MASK | LAYER_MASK | MTYPE_MASK | MSIZE_MASK, go1);
    EMAddGraphicsToModel(ESIModel(), go1);

    // take into account periodic conditions
    if ( giveNumberOfNodes() == 3 ) {
        Node *nodeC = ( Particle * ) giveNode(3);
        p [ 1 ].x += kxa + kxa * defScale * ( nodeC->giveDofWithID(D_u)->giveUnknown(VM_Total, tStep) ) + kyb * defScale * ( nodeC->giveDofWithID(R_u)->giveUnknown(VM_Total, tStep) );
        p [ 1 ].y += kyb + kyb * defScale * ( nodeC->giveDofWithID(D_v)->giveUnknown(VM_Total, tStep) ) + kzc * defScale * ( nodeC->giveDofWithID(R_v)->giveUnknown(VM_Total, tStep) );
        p [ 1 ].z += kzc + kzc * defScale * ( nodeC->giveDofWithID(D_w)->giveUnknown(VM_Total, tStep) ) + kxa * defScale * ( nodeC->giveDofWithID(R_w)->giveUnknown(VM_Total, tStep) );
        EASValsSetMType(CIRCLE_MARKER);
    }

    // plot the second particle
    go2 = CreateMarker3D(p + 1);
    EGWithMaskChangeAttributes(COLOR_MASK | LAYER_MASK | MTYPE_MASK | MSIZE_MASK, go2);
    EMAddGraphicsToModel(ESIModel(), go2);
}


void
CohesiveSurface3d :: drawScalar(oofegGraphicContext &gc, TimeStep *tStep)
{
    if ( !gc.testElementGraphicActivity(this) ) {
        return;
    }

    FloatArray val;
    GaussPoint *gp = integrationRulesArray [ 0 ]->getIntegrationPoint(0);
    if ( !giveIPValue(val, gp, gc.giveIntVarType(), tStep) ) {
        return;
    }

    int indx = gc.giveIntVarIndx();

    double s [ 8 ];
    for ( int i = 0; i < 8; i++ ) {
        s [ i ] = val.at(indx);
    }

    gc.updateFringeTableMinMax(s, 1);

    WCRec p [ 8 ];
    Particle *nodeA = ( Particle * ) giveNode(1);
    Particle *nodeB = ( Particle * ) giveNode(2);
    if ( gc.getInternalVarsDefGeoFlag() ) {
        // use deformed geometry
        double defScale = gc.getDefScale();
        p [ 0 ].x = nodeA->giveUpdatedCoordinate(1, tStep, defScale);
        p [ 0 ].y = nodeA->giveUpdatedCoordinate(2, tStep, defScale);
        p [ 0 ].z = nodeA->giveUpdatedCoordinate(3, tStep, defScale);
        p [ 2 ].x = nodeB->giveUpdatedCoordinate(1, tStep, defScale);
        p [ 2 ].y = nodeB->giveUpdatedCoordinate(2, tStep, defScale);
        p [ 2 ].z = nodeB->giveUpdatedCoordinate(3, tStep, defScale);
        // handle special elements crossing the boundary of the periodic cell
        if ( giveNumberOfNodes() == 3 ) {
            Node *nodeC = ( Particle * ) giveNode(3);
            p [ 2 ].x += kxa + kxa * defScale * ( nodeC->giveDofWithID(D_u)->giveUnknown(VM_Total, tStep) ) + kyb * defScale * ( nodeC->giveDofWithID(R_u)->giveUnknown(VM_Total, tStep) );
            p [ 2 ].y += kyb + kyb * defScale * ( nodeC->giveDofWithID(D_v)->giveUnknown(VM_Total, tStep) ) + kzc * defScale * ( nodeC->giveDofWithID(R_v)->giveUnknown(VM_Total, tStep) );
            p [ 2 ].z += kzc + kzc * defScale * ( nodeC->giveDofWithID(D_w)->giveUnknown(VM_Total, tStep) ) + kxa * defScale * ( nodeC->giveDofWithID(R_w)->giveUnknown(VM_Total, tStep) );
        }
    } else {
        // use initial geometry
        p [ 0 ].x = nodeA->giveCoordinate(1);
        p [ 0 ].y = nodeA->giveCoordinate(2);
        p [ 0 ].z = nodeA->giveCoordinate(3);
        p [ 2 ].x = nodeB->giveCoordinate(1);
        p [ 2 ].y = nodeB->giveCoordinate(2);
        p [ 2 ].z = nodeB->giveCoordinate(3);
        // handle special elements crossing the boundary of the periodic cell
        if ( giveNumberOfNodes() == 3 ) {
            p [ 2 ].x += kxa;
            p [ 2 ].y += kyb;
            p [ 2 ].z += kzc;
        }
    }


    double r1 = nodeA->giveRadius();
    double r2 = nodeB->giveRadius();
    double d = 0.1 * ( r1 + r2 );
    p [ 1 ].x = 0.5 * ( p [ 0 ].x + p [ 2 ].x - d * lcs.at(2, 1) - d * lcs.at(3, 1) );
    p [ 1 ].y = 0.5 * ( p [ 0 ].y + p [ 2 ].y - d * lcs.at(2, 2) - d * lcs.at(3, 2) );
    p [ 1 ].z = 0.5 * ( p [ 0 ].z + p [ 2 ].z - d * lcs.at(2, 3) - d * lcs.at(3, 3) );
    p [ 3 ].x = p [ 1 ].x + d *lcs.at(2, 1);
    p [ 3 ].y = p [ 1 ].y + d *lcs.at(2, 2);
    p [ 3 ].z = p [ 1 ].z + d *lcs.at(2, 3);

    for ( int i = 5; i < 8; i += 2 ) {
        p [ i ].x = p [ i - 4 ].x + d *lcs.at(3, 1);
        p [ i ].y = p [ i - 4 ].y + d *lcs.at(3, 2);
        p [ i ].z = p [ i - 4 ].z + d *lcs.at(3, 3);
    }

    p [ 4 ] = p [ 0 ];
    p [ 6 ] = p [ 2 ];

    GraphicObj *go = CreateHexahedronWD(p, s);

    EASValsSetLayer(OOFEG_VARPLOT_PATTERN_LAYER);
    EASValsSetLineWidth(2 * OOFEG_DEFORMED_GEOMETRY_WIDTH);
    EASValsSetFillStyle(FILL_SOLID);

    //EGWithMaskChangeAttributes(WIDTH_MASK | COLOR_MASK | LAYER_MASK, go);
    EGWithMaskChangeAttributes(WIDTH_MASK | FILL_MASK | LAYER_MASK, go);
    EMAddGraphicsToModel(ESIModel(), go);
}
#endif
} // namespace oofem
