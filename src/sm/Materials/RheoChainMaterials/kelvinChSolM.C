/*
 *
 *                 #####    #####   ######  ######  ###   ###
 *               ##   ##  ##   ##  ##      ##      ## ### ##
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

#include "mathfem.h"
#include "kelvinChSolM.h"
#include "floatarray.h"
#include "floatmatrix.h"
#include "gausspoint.h"
#include "timestep.h"
#include "contextioerr.h"


namespace oofem {
KelvinChainSolidMaterial :: KelvinChainSolidMaterial(int n, Domain *d) : RheoChainMaterial(n, d)
{ }

double
KelvinChainSolidMaterial :: giveEModulus(GaussPoint *gp, TimeStep *tStep) const
{
    /*
     * This function returns the incremental modulus for the given time increment.
     * Return value is the incremental E modulus of non-aging Kelvin chain without the first unit (elastic spring)
     * The modulus may also depend on the specimen geometry (gp - dependence).
     *
     * It is stored as "Einc" for further expected requests from other gaussPoints that correspond to the same material.
     *
     * Note: time -1 refers to the previous time.
     */

    if ( ! Material :: isActivated( tStep ) ) {
        OOFEM_ERROR("Attempted to evaluate E modulus at time lower than casting time");
    }

    if ( this->EparVal.isEmpty() ) {
        this->updateEparModuli(0., gp, tStep); // stiffnesses are time independent (evaluated at time t = 0.)
    }

    double sum = 0.0;
    for ( int mu = 1; mu <= nUnits; mu++ ) {
        double lambdaMu = this->computeLambdaMu(gp, tStep, mu);
        double Emu = this->giveEparModulus(mu);
        sum += ( 1 - lambdaMu ) / Emu;
    }

    double v = this->computeSolidifiedVolume(gp, tStep);
    return sum / v;
}

void
KelvinChainSolidMaterial :: giveEigenStrainVector(FloatArray &answer, GaussPoint *gp, TimeStep *tStep, ValueModeType mode) const
//
// computes the strain due to creep at constant stress during the increment
// (in fact, the INCREMENT of creep strain is computed for mode == VM_Incremental)
//
{
    KelvinChainSolidMaterialStatus *status = static_cast< KelvinChainSolidMaterialStatus * >( this->giveStatus(gp) );

    if ( ! Material :: isActivated( tStep ) ) {
        OOFEM_ERROR("Attempted to evaluate creep strain for time lower than casting time");
    }

    if ( this->EparVal.isEmpty() ) {
        this->updateEparModuli(0., gp, tStep); // stiffnesses are time independent (evaluated at time t = 0.)
    }

    if ( mode == VM_Incremental ) {
        FloatArray *sigmaVMu = nullptr, reducedAnswer;
        for ( int mu = 1; mu <= nUnits; mu++ ) {
            double betaMu = this->computeBetaMu(gp, tStep, mu);
            sigmaVMu = & status->giveHiddenVarsVector(mu); // JB

            if ( sigmaVMu->isNotEmpty() ) {
                reducedAnswer.add(( 1.0 - betaMu ) / this->giveEparModulus(mu), * sigmaVMu);
            }
        }

        if ( sigmaVMu->isNotEmpty() ) {
            FloatMatrix C;
            FloatArray help = reducedAnswer;
            this->giveUnitComplianceMatrix(C, gp, tStep);
            reducedAnswer.beProductOf(C, help);
            double v = this->computeSolidifiedVolume(gp, tStep);
            reducedAnswer.times(1. / v);
        }

        answer = reducedAnswer;
    } else {
        /* error - total mode not implemented yet */
        OOFEM_ERROR("mode is not supported");
    }
}

double
KelvinChainSolidMaterial :: computeBetaMu(GaussPoint *gp, TimeStep *tStep, int Mu) const
{
    double deltaT = tStep->giveTimeIncrement();
    double tauMu = this->giveCharTime(Mu);

    if ( deltaT / tauMu > 30 ) {
        return 0;
    } else {
        return exp(-( deltaT ) / tauMu);
    }
}

double
KelvinChainSolidMaterial :: computeLambdaMu(GaussPoint *gp, TimeStep *tStep, int Mu) const
{
    double deltaT = tStep->giveTimeIncrement();
    double tauMu = this->giveCharTime(Mu);

    if ( deltaT / tauMu < 1.e-5 ) {
        return 1 - 0.5 * ( deltaT / tauMu ) + 1 / 6 * ( pow(deltaT / tauMu, 2) ) - 1 / 24 * ( pow(deltaT / tauMu, 3) );
    } else if ( deltaT / tauMu > 30 ) {
        return tauMu / deltaT;
    } else {
        return ( 1.0 -  exp(-( deltaT ) / tauMu) ) * tauMu / deltaT;
    }
}


void
KelvinChainSolidMaterial :: giveRealStressVector(FloatArray &answer, GaussPoint *gp, const FloatArray &reducedStrain, TimeStep *tStep)
{
    RheoChainMaterial :: giveRealStressVector(answer, gp, reducedStrain, tStep);

    // Computes hidden variables and stores them as temporary
    this->computeHiddenVars(gp, tStep);
}


void
KelvinChainSolidMaterial :: computeHiddenVars(GaussPoint *gp, TimeStep *tStep)
{
    /*
     * Updates hidden variables used to effectively trace the load history
     */

    FloatArray help, SigmaVMu, deltaSigma;
    FloatMatrix D;
    KelvinChainSolidMaterialStatus *status = static_cast< KelvinChainSolidMaterialStatus * >( this->giveStatus(gp) );

    // goes there if the viscoelastic material does not exist and at the same time the precastingtime mat is not provided
    //    if (  ! this->isActivated( tStep ) )  {
    // goes there if the viscoelastic material does not exist yet
    if (  ! Material :: isActivated( tStep ) )  {
        help.resize(StructuralMaterial :: giveSizeOfVoigtSymVector( gp->giveMaterialMode() ) );
        help.zero();
        for ( int mu = 1; mu <= nUnits; mu++ ) {
            status->letTempHiddenVarsVectorBe(mu, help);
        }
        return;
    }
    
    help = status->giveTempStrainVector(); // gives updated strain vector (at the end of time-step)
    help.subtract( status->giveStrainVector() ); // strain increment in current time-step

    // Subtract the stress-independent part of strain
    auto deltaEps0 = this->computeStressIndependentStrainVector(gp, tStep, VM_Incremental);
    if ( deltaEps0.giveSize() ) {
        help.subtract(deltaEps0); // should be equal to zero if there is no stress change during the time-step
    }

    this->giveUnitStiffnessMatrix(D, gp, tStep);

    help.times( this->giveEModulus(gp, tStep) );
    // help.times( this->giveIncrementalModulus(gp, tStep) );
    deltaSigma.beProductOf(D, help);

    for ( int mu = 1; mu <= nUnits; mu++ ) {
        double betaMu = this->computeBetaMu(gp, tStep, mu);
        double lambdaMu = this->computeLambdaMu(gp, tStep, mu);

        help = deltaSigma;
        help.times(lambdaMu);

        SigmaVMu = status->giveHiddenVarsVector(mu);

        if ( SigmaVMu.giveSize() ) {
            SigmaVMu.times(betaMu);
            SigmaVMu.add(help);
            status->letTempHiddenVarsVectorBe(mu, SigmaVMu);
        } else {
            status->letTempHiddenVarsVectorBe(mu, help);
        }
    }
}


MaterialStatus *
KelvinChainSolidMaterial :: CreateStatus(GaussPoint *gp) const
/*
 * creates a new material status corresponding to this class
 */
{
    return new KelvinChainSolidMaterialStatus(gp, nUnits);
}

void
KelvinChainSolidMaterial :: initializeFrom(InputRecord &ir)
{
    RheoChainMaterial :: initializeFrom(ir);
}

// useless here
double
KelvinChainSolidMaterial :: computeCreepFunction(double t, double t_prime, GaussPoint *gp, TimeStep *tStep) const
{
    OOFEM_ERROR("function has not been yet implemented to KelvinChainSolidMaterialStatus.C");
}


/****************************************************************************************/

KelvinChainSolidMaterialStatus :: KelvinChainSolidMaterialStatus(GaussPoint *g, int nunits) :
    RheoChainMaterialStatus(g, nunits) { }

void
KelvinChainSolidMaterialStatus :: printOutputAt(FILE *file, TimeStep *tStep) const
{
    RheoChainMaterialStatus :: printOutputAt(file, tStep);
}


void
KelvinChainSolidMaterialStatus :: updateYourself(TimeStep *tStep)
{
    RheoChainMaterialStatus :: updateYourself(tStep);
}

void
KelvinChainSolidMaterialStatus :: initTempStatus()
{
    RheoChainMaterialStatus :: initTempStatus();
}

void
KelvinChainSolidMaterialStatus :: saveContext(DataStream &stream, ContextMode mode)
{
    RheoChainMaterialStatus :: saveContext(stream, mode);
}

void
KelvinChainSolidMaterialStatus :: restoreContext(DataStream &stream, ContextMode mode)
{
    RheoChainMaterialStatus :: restoreContext(stream, mode);
}
} // end namespace oofem
