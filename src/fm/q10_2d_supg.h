/* $Header: /home/cvs/bp/oofem/tm/src/transportelement.h,v 1.3 2003/04/23 14:22:15 bp Exp $ */
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
 *               Copyright (C) 1993 - 2008   Borek Patzak
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

#ifndef q10_2d_supg_h
#define q10_2d_supg_h

#include "supgelement2.h"
#include "flotmtrx.h"
#include "fei2dquadlin.h"
#include "fei2dquadconst.h"

#include "primaryfield.h"
#include "spatiallocalizer.h"
#include "zznodalrecoverymodel.h"
#include "nodalaveragingrecoverymodel.h"
#include "sprnodalrecoverymodel.h"
#include "leplic.h"
#include "levelsetpcs.h"
#include "elementinternaldofman.h"

namespace oofem {

/**
 * Class representing 2d triangular element  with quadratic velocity
 * and linear pressure approximations for solving incompressible fluid problems
 * with SUPG solver
 *
 */
class Q10_2D_SUPG : public SUPGElement2, public LevelSetPCSElementInterface, public ZZNodalRecoveryModelInterface, public NodalAveragingRecoveryModelInterface
{
protected:
    static FEI2dQuadLin velocityInterpolation;
    static FEI2dQuadConst pressureInterpolation;
    ElementDofManager pressureNode;

public:
    Q10_2D_SUPG(int n, Domain *d);
    ~Q10_2D_SUPG();

    // definition
    const char *giveClassName() const { return "Q10_2D_SUPG"; }
    classType giveClassID() const { return SUPGElementClass; }
    Element_Geometry_Type giveGeometryType() const { return EGT_quad_1; }
    MaterialMode giveMaterialMode() { return _2dFlow; }

    virtual void giveInternalDofManDofIDMask(int i, EquationID, IntArray & answer) const;
    virtual void giveDofManDofIDMask(int inode, EquationID ut, IntArray &answer) const;
    virtual int computeNumberOfDofs(EquationID ut);
    IRResultType initializeFrom(InputRecord *ir);
    virtual void updateYourself(TimeStep *tStep);
    virtual int checkConsistency();

    contextIOResultType saveContext(DataStream *stream, ContextMode mode, void *obj = NULL);
    contextIOResultType restoreContext(DataStream *stream, ContextMode mode, void *obj = NULL);

    virtual double LS_PCS_computeF(LevelSetPCS *ls, TimeStep *tStep);
    virtual void LS_PCS_computedN(FloatMatrix &answer);
    virtual double LS_PCS_computeVolume() {return 0.0;};
    virtual double LS_PCS_computeS(LevelSetPCS *ls, TimeStep *tStep);
    virtual void LS_PCS_computeVOFFractions(FloatArray &answer, FloatArray &fi);

    Interface *giveInterface(InterfaceType t);

    int ZZNodalRecoveryMI_giveDofManRecordSize(InternalStateType type);
    Element *ZZNodalRecoveryMI_giveElement() { return this; }
    void ZZNodalRecoveryMI_ComputeEstimatedInterpolationMtrx(FloatMatrix &answer, GaussPoint *aGaussPoint,
                                                             InternalStateType type);

    void NodalAveragingRecoveryMI_computeNodalValue(FloatArray &answer, int node,
                                                    InternalStateType type, TimeStep *tStep);
    virtual int NodalAveragingRecoveryMI_giveDofManRecordSize(InternalStateType type)
    { return ZZNodalRecoveryMI_giveDofManRecordSize(type); }
    void NodalAveragingRecoveryMI_computeSideValue(FloatArray &answer, int side,
						   InternalStateType type, TimeStep *tStep);

    /// @name Helping functions for computing VOFFractions.
    //@{
    void computeIntersection(int iedge, FloatArray &intcoords, FloatArray &fi);

    void computeMiddlePointOnParabolicArc(FloatArray &answer, int iedge, FloatArray borderpoints);

    void computeCenterOf(FloatArray &C, FloatArray c, int dim);

    void computeQuadraticRoots(FloatArray Coeff, double &r1, double &r2);

    void computeCoordsOfEdge(FloatArray &answer, int iedge);

    void computeQuadraticFunct(FloatArray &answer, int iedge);

    void computeQuadraticFunct(FloatArray &answer, FloatArray line);
    //@}

    virtual int giveIPValue(FloatArray &answer, GaussPoint *gp, InternalStateType type, TimeStep *tStep);
    virtual int giveIPValueSize(InternalStateType type, GaussPoint *gp);
    virtual InternalStateValueType giveIPValueType(InternalStateType type);
    virtual int giveIntVarCompFullIndx(IntArray &answer, InternalStateType type);

#ifdef __OOFEG
    int giveInternalStateAtNode(FloatArray &answer, InternalStateType type, InternalStateMode mode,
                                int node, TimeStep *atTime);
    // Graphics output
    //void drawYourself (oofegGraphicContext&);
    virtual void drawRawGeometry(oofegGraphicContext &);
    virtual void drawScalar(oofegGraphicContext &context);
    //virtual void drawDeformedGeometry(oofegGraphicContext&, UnknownType) {}
#endif

    virtual void printOutputAt(FILE *file, TimeStep *tStep);
    double computeCriticalTimeStep(TimeStep *tStep);

    // three terms for computing their norms due to computing t_supg
    virtual void computeAdvectionTerm(FloatMatrix &answer, TimeStep *atTime);
    virtual void computeAdvectionDeltaTerm(FloatMatrix &answer, TimeStep *atTime);
    virtual void computeMassDeltaTerm(FloatMatrix &answer, TimeStep *atTime);
    virtual void computeLSICTerm(FloatMatrix &answer, TimeStep *atTime);
    virtual void computeAdvectionEpsilonTerm(FloatMatrix &answer, TimeStep *atTime);
    virtual void computeMassEpsilonTerm(FloatMatrix &answer, TimeStep *atTime);

    //virtual int giveNumberOfDofs() { return 1; }
    virtual int giveNumberOfInternalDofManagers() {return 1;}
    virtual DofManager *giveInternalDofManager(int i) const ;
      //_error2("No such DOF available on Element %d", number);
      //return pressureDof(dofID P);
      //}*/

    virtual void giveLocationArray(IntArray &locationArray, EquationID, const UnknownNumberingScheme &s) const;

protected:
    virtual void giveLocalVelocityDofMap (IntArray &map);
    virtual void giveLocalPressureDofMap (IntArray &map);

    void computeGaussPoints();
    virtual void computeNuMatrix(FloatMatrix &answer, GaussPoint *gp);
    virtual void computeUDotGradUMatrix(FloatMatrix &answer, GaussPoint *gp, TimeStep *atTime);
    virtual void computeBMatrix(FloatMatrix &anwer, GaussPoint *gp);
    virtual void computeDivUMatrix(FloatMatrix &answer, GaussPoint *gp);
    virtual void computeNpMatrix(FloatMatrix &answer, GaussPoint *gp);
    virtual void computeGradPMatrix(FloatMatrix &answer, GaussPoint *gp);
    virtual void computeDivTauMatrix(FloatMatrix &answer, GaussPoint *gp, TimeStep *atTime);
    virtual void computeGradUMatrix(FloatMatrix &answer, GaussPoint *gp, TimeStep *atTime);
    virtual int giveNumberOfSpatialDimensions();
    double computeVolumeAround(GaussPoint *gp);

    virtual void updateStabilizationCoeffs(TimeStep *tStep);

    virtual int giveTermIntergationRuleIndex(CharType termType);
};

} // end namespace oofem
#endif // q10_2d_supg_h
