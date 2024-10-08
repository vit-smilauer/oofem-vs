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

#include "masonry02.h"
#include "sm/Materials/isolinearelasticmaterial.h"
#include "gausspoint.h"
#include "floatmatrix.h"
#include "floatarray.h"
#include "intarray.h"
#include "mathfem.h"
#include "classfactory.h"

namespace oofem {
REGISTER_Material(Masonry02);

Masonry02 :: Masonry02(int n, Domain *d) : MPlasticMaterial2(n, d)
{
    linearElasticMaterial = new IsotropicLinearElasticMaterial(n, d);
    this->nsurf = 3;
    //this->rmType = mpm_CuttingPlane;
    this->rmType = mpm_ClosestPoint;
    this->plType = nonassociatedPT;
}


bool
Masonry02 :: hasMaterialModeCapability(MaterialMode mode) const
//
// returns whether receiver supports given mode
//
{
    return mode == _2dInterface;
}

void
Masonry02 :: initializeFrom(InputRecord &ir)
{
    MPlasticMaterial2 :: initializeFrom(ir);
    linearElasticMaterial->initializeFrom(ir);

    IR_GIVE_FIELD(ir, ft0, _IFT_Masonry02_ft0);
    IR_GIVE_FIELD(ir, gfI, _IFT_Masonry02_gfi);
    IR_GIVE_FIELD(ir, gfII, _IFT_Masonry02_gfii);
    IR_GIVE_FIELD(ir, kn, _IFT_Masonry02_kn);
    IR_GIVE_FIELD(ir, ks, _IFT_Masonry02_ks);
    IR_GIVE_FIELD(ir, c0, _IFT_Masonry02_c0);
    IR_GIVE_FIELD(ir, tanfi0, _IFT_Masonry02_tanfi0);
    IR_GIVE_FIELD(ir, tanfir, _IFT_Masonry02_tanfir);
    IR_GIVE_FIELD(ir, tanpsi, _IFT_Masonry02_tanpsi);

    // cap mode params
    // IR_GIVE_FIELD (ir, fm, "fm");
    IR_GIVE_FIELD(ir, Cnn, _IFT_Masonry02_cnn);
    IR_GIVE_FIELD(ir, Css, _IFT_Masonry02_css);
    IR_GIVE_FIELD(ir, Cn, _IFT_Masonry02_cn);

    IR_GIVE_FIELD(ir, sic, _IFT_Masonry02_si);
    IR_GIVE_FIELD(ir, spc, _IFT_Masonry02_sp);
    IR_GIVE_FIELD(ir, smc, _IFT_Masonry02_sm);
    IR_GIVE_FIELD(ir, src, _IFT_Masonry02_sr);
    IR_GIVE_FIELD(ir, kp, _IFT_Masonry02_kp);
    IR_GIVE_FIELD(ir, km, _IFT_Masonry02_km);
    IR_GIVE_FIELD(ir, kr, _IFT_Masonry02_kr);

    if ( ir.hasField(_IFT_Masonry02_cplane) ) {
        this->rmType = mpm_CuttingPlane;
    }
}


double
Masonry02 :: computeYieldValueAt(GaussPoint *gp, int isurf, const FloatArray &stressVector,
                                 const FloatArray &strainSpaceHardeningVariables) const
{
    if ( isurf == 1 ) {
        // tension mode
        // double help = this->gfI*this->c0/(this->gfII*this->ft0);
        double k1 = strainSpaceHardeningVariables.at(1);
        //double ft = min( this->ft0*exp((-1.0)*this->ft0*k1/this->gfI), this->ft0);
        double ft = this->ft0 * exp( ( -1.0 ) * this->ft0 * k1 / this->gfI );
        return stressVector.at(1) - ft;
    } else if ( isurf == 2 ) {
        // double help = this->gfI*this->c0/(this->gfII*this->ft0);
        double k2 = strainSpaceHardeningVariables.at(2);
        //double c=   min (this->c0*exp((-1.0)*this->c0*k2/this->gfII), this->c0);

        double c = this->c0 * exp( ( -1.0 ) * this->c0 * k2 / this->gfII );
        double tanfi = tanfi0 + ( tanfir - tanfi0 ) * ( c0 - c ) / c0;
        //double nx = tanfi; double ny=sgn (stressVector.at(2));

        return fabs( stressVector.at(2) ) + stressVector.at(1) * tanfi - c;

        /*
         * // test fan region
         * if (((nx*(stressVector.at(1)-c) + ny*stressVector.at(2)) > 0.0) &&
         *  ((nx*(stressVector.at(1)-c) - ny*stressVector.at(2)) > 0.0)) {
         * // fan region active
         * return (stressVector.at(1)-c)*(stressVector.at(1)-c)+stressVector.at(2)*stressVector.at(2);
         * } else {
         * // shear mode
         * double tanfi=tanfi0+(tanfir-tanfi0)*(c0-c)/c0;
         * return fabs(stressVector.at(2))+stressVector.at(1)*tanfi-c;
         * }
         */
    } else if ( isurf == 3 ) {
        double s1 = stressVector.at(1);
        double s2 = stressVector.at(2);
        double st = this->computeF3HardeningLaw( strainSpaceHardeningVariables.at(3) );
        return this->Cnn * s1 * s1 + this->Css * s2 * s2 + this->Cn * s1 - st * st;
    } else {
        return 0.0;
    }
}

void
Masonry02 :: computeStressGradientVector(FloatArray &answer, functType ftype, int isurf, GaussPoint *gp, const FloatArray &stressVector,
                                         const FloatArray &strainSpaceHardeningVariables) const
{
    answer.resize(2);
    answer.zero();

    if ( isurf == 1 ) {
        answer.at(1) = 1.0;
    } else if ( isurf == 2 ) {
        if ( ftype == yieldFunction ) {
            // double help = this->gfI*this->c0/(this->gfII*this->ft0);
            double k2 = strainSpaceHardeningVariables.at(2);
            double c =   this->c0 * exp( ( -1.0 ) * this->c0 * k2 / this->gfII );

            double tanfi = tanfi0 + ( tanfir - tanfi0 ) * ( c0 - c ) / c0;
            double nx = tanfi;
            double ny = sgn( stressVector.at(2) );

            answer.at(1) = nx;
            answer.at(2) = ny;
        } else {
            //double k2 = strainSpaceHardeningVariables.at(2);
            //double c=   this->c0*exp((-1.0)*this->c0*k2/this->gfII);

            answer.at(1) = tanpsi;
            answer.at(2) = sgn( stressVector.at(2) );
        }

        /*
         * // test fan region
         * if (((nx*(stressVector.at(1)-c) + ny*stressVector.at(2)) > 0.0) &&
         *    ((nx*(stressVector.at(1)-c) - ny*stressVector.at(2)) > 0.0)) {
         *  // fan region active
         *  answer.at(1) = 2.0*(stressVector.at(1)-c);
         *  answer.at(2) = 2.0*stressVector.at(2);
         *  if (fabs(answer.at(2))<1.e-8) answer.at(2) = (answer.at(1))*1.e-6;
         * } else {
         *  answer.at(1) = nx;
         *  answer.at(2) = ny;
         * }
         */
    } else if ( isurf == 3 ) {
        answer.at(1) = 2.0 *this->Cnn *stressVector.at(1) + this->Cn;
        answer.at(2) = 2.0 *this->Css *stressVector.at(2);
    }
}


//////////////////////////////////////////////////
// up to here done
//////////////////////////////////////////////////

void
Masonry02 :: computeStrainHardeningVarsIncrement(FloatArray &answer, GaussPoint *gp,
                                                 const FloatArray &stress, const FloatArray &dlambda,
                                                 const FloatArray &dplasticStrain, const IntArray &activeConditionMap) const
{
    double help = this->gfI * this->c0 / ( this->gfII * this->ft0 );

    answer.resize(3);
    answer.zero();

    if ( activeConditionMap.at(1) && activeConditionMap.at(2) ) {
        if ( ( dlambda.at(1) > 0. ) && ( dlambda.at(2) > 0. ) ) {
            answer.at(1) = sqrt( dlambda.at(1) * dlambda.at(1) + ( help * dlambda.at(2) ) * ( help * dlambda.at(2) ) );
            answer.at(2) = sqrt( ( dlambda.at(1) / help ) * ( dlambda.at(1) / help ) + dlambda.at(2) * dlambda.at(2) );
        } else if ( dlambda.at(1) > 0. ) {
            answer.at(1) = dlambda.at(1);
            answer.at(2) = dlambda.at(1) / help;
        } else if ( dlambda.at(2) > 0. ) {
            answer.at(1) = help * dlambda.at(2);
            answer.at(2) = dlambda.at(2);
        }
    } else if ( activeConditionMap.at(1) ) {
        if ( dlambda.at(1) > 0. ) {
            answer.at(1) = dlambda.at(1);
            answer.at(2) = dlambda.at(1) / help;
        }
    } else {
        if ( dlambda.at(2) > 0. ) {
            answer.at(1) = help * dlambda.at(2);
            answer.at(2) = dlambda.at(2);
        }
    }

    double p1 = 2.0 *this->Cnn *stress.at(1) + this->Cn;
    double p2 = 2.0 *this->Css *stress.at(2);

    if ( dlambda.at(3) > 0. ) {
        answer.at(3) = dlambda.at(3) * sqrt(p1 * p1 + p2 * p2);
    }

    /*
     * double l1,l2;
     * l1 = max (dlambda.at(1), 0.);
     * l2 = max (dlambda.at(2), 0.);
     *
     * answer.at(1) =sqrt(l1*l1+(help*l2)*(help*l2));
     * answer.at(2) =sqrt((l1/help)*(l1/help)+l2*l2);
     *
     *
     * double p1 = 2.0*this->Cnn*stress.at(1)+this->Cn;
     * double p2 = 2.0*this->Css*stress.at(2);
     * if (dlambda.at(3) <=0.) answer.at(3) = 0.;
     * else answer.at(3) = dlambda.at(3)*sqrt(p1*p1+p2*p2);
     */
}

void
Masonry02 :: computeReducedHardeningVarsLamGradient(FloatMatrix &answer, GaussPoint *gp, int actSurf,
                                                    const IntArray &activeConditionMap,
                                                    const FloatArray &fullStressVector,
                                                    const FloatArray &strainSpaceHardeningVars,
                                                    const FloatArray &dlambda) const
{
    // computes dk(i)/dLambda(j)
    int indx;
    answer.resize(3, actSurf);
    answer.zero();

    double help = this->gfI * this->c0 / ( this->gfII * this->ft0 );
    double k1 = sqrt( dlambda.at(1) * dlambda.at(1) +
                     ( help * dlambda.at(2) ) * ( help * dlambda.at(2) ) );
    double k2 = sqrt( ( dlambda.at(1) / help ) * ( dlambda.at(1) / help ) +
                     dlambda.at(2) * dlambda.at(2) );

    double p1 = 2.0 *this->Cnn *fullStressVector.at(1) + this->Cn;
    double p2 = 2.0 *this->Css *fullStressVector.at(2);

    if ( ( indx = activeConditionMap.at(1) ) ) {
        if ( dlambda.at(1) > 0. ) {
            if ( activeConditionMap.at(2) ) {
                if ( k1 > 0.0 ) {
                    double dl1 = dlambda.at(1);
                    answer.at(1, indx) = dl1 / k1; //dk1/dl1
                    answer.at(2, indx) = dl1 / k2 / help / help; //dk2/dl1
                } else {
                    // derivative with respect to l1 when l1=l2=0.0 and both functions active
                    answer.at(1, indx) = 0.;
                    answer.at(2, indx) = 0.;
                }
            } else {
                // derivative with respect to l1 when only function 1 active (l2 always 0)
                answer.at(1, indx) = 1.;
                answer.at(2, indx) = 0.0;
                //answer.at(2,indx)=1./help;
                /*
                 * answer.at(1,indx)= 1.0;     // added 1.e4
                 * answer.at(2,indx)= 1.0/help;
                 */
            }
        }
    }

    if ( ( indx = activeConditionMap.at(2) ) ) {
        if ( dlambda.at(2) > 0. ) {
            if ( activeConditionMap.at(1) ) {
                if ( k1 > 0.0 ) {
                    double dl2 = dlambda.at(2);
                    answer.at(1, indx) = help * help * dl2 / k1;
                    answer.at(2, indx) = dl2 / k2;
                } else {
                    // derivative with respect to l2 when l1=l2=0.0 and both functions active
                    answer.at(1, indx) = 0.;
                    answer.at(2, indx) = 0.;
                }
            } else {
                // derivative with respect to l2 when and only function 2 active (l1 always 0)
                answer.at(1, indx) = 0.0;
                //answer.at(1,indx)= 1.*help;
                answer.at(2, indx) = 1.0;
            }
        }
    }

    if ( ( indx = activeConditionMap.at(3) ) ) {
        if ( dlambda.at(3) < 0. ) {
            answer.at(3, indx) = 0.0;
        } else {
            answer.at(3, indx) = sqrt(p1 * p1 + p2 * p2);
        }
    }
}




void
Masonry02 :: computeKGradientVector(FloatArray &answer, functType ftype, int isurf, GaussPoint *gp, FloatArray &stressVector,
                                    const FloatArray &strainSpaceHardeningVariables) const
{
    answer.resize(3);

    if ( isurf == 1 ) {
        answer.at(1) = ( -1.0 ) * this->ft0 * exp( ( -1.0 ) * this->ft0 * strainSpaceHardeningVariables.at(1) / this->gfI ) * ( -1.0 ) * this->ft0 / this->gfI;
        answer.at(2) = 0.0;
        answer.at(3) = 0.0;
    } else if ( isurf == 2 ) {
        if ( ftype == yieldFunction ) {
            //double help = this->gfI*this->c0/(this->gfII*this->ft0);
            double k2 = strainSpaceHardeningVariables.at(2);
            double c =   this->c0 * exp( ( -1.0 ) * this->c0 * k2 / this->gfII );


            //double tanfi=tanfi0+(tanfir-tanfi0)*(c0-c)/c0;
            //double nx = tanfi; double ny=sgn (stressVector.at(2));

            answer.at(1) = 0.0;
            answer.at(2) = ( -1.0 ) * ( stressVector.at(1) * ( tanfir - tanfi0 ) / c0 + 1.0 ) * c * ( -1.0 ) * this->c0 / this->gfII;
            answer.at(3) = 0.0;
        } else {
            double k2 = strainSpaceHardeningVariables.at(2);
            double c =   this->c0 * exp( ( -1.0 ) * this->c0 * k2 / this->gfII );

            answer.at(1) = 0.0;
            answer.at(2) = ( -1.0 ) * c * ( -1.0 ) * this->c0 / this->gfII;
            answer.at(3) = 0.0;
        }

        /*
         * // test fan region
         * if (((nx*(stressVector.at(1)-c) + ny*stressVector.at(2)) > 0.0) &&
         *  ((nx*(stressVector.at(1)-c) - ny*stressVector.at(2)) > 0.0)) {
         * // fan region active
         * answer.at(1) = 2.0*stressVector.at(2);
         * answer.at(2) = 2.0*(stressVector.at(1)-c);
         * } else {
         * answer.at(1) = 0.0;
         * answer.at(2) = (-1.0)*(stressVector.at(1)*(tanfir-tanfi0)/c0+1.0)*c*(-1.0)*this->c0/this->gfII;
         * }
         */
    } else if ( isurf == 3 ) {
        double st, gt;
        st = this->computeF3HardeningLaw( strainSpaceHardeningVariables.at(3) );
        gt = this->computeF3HardeningGradient( strainSpaceHardeningVariables.at(3) );
        answer.at(1) = 0.0;
        answer.at(2) = 0.0;
        answer.at(3) = -2.0 * st * gt;
    }
}

void
Masonry02 :: computeReducedHardeningVarsSigmaGradient(FloatMatrix &answer, GaussPoint *gp, const IntArray &activeConditionMap,
                                                      const FloatArray &fullStressVector,
                                                      const FloatArray &strainSpaceHardeningVars,
                                                      const FloatArray &dlambda) const
{
    // computes dk(i)/dsig(j) gradient matrix
    answer.resize(3, 2);
    answer.zero();

    double p1 = 2.0 *this->Cnn *fullStressVector.at(1) + this->Cn;
    double p2 = 2.0 *this->Css *fullStressVector.at(2);

    if ( activeConditionMap.at(3) ) {
        if ( dlambda.at(3) >= 0. ) {
            double c = 0.5 * dlambda.at(3) / sqrt(p1 * p1 + p2 * p2);
            answer.at(3, 1) = 4.0 * c * this->Cnn * p1;
            answer.at(3, 2) = 4.0 * c * this->Css * p2;
        }
    }
}



void
Masonry02 :: computeReducedSSGradientMatrix(FloatMatrix &gradientMatrix,  int i, GaussPoint *gp, const FloatArray &fullStressVector,
                                            const FloatArray &strainSpaceHardeningVariables) const
{
    // computes second derivatives of load fuction with respect to stresses
    gradientMatrix.resize(2, 2);
    gradientMatrix.zero();

    if ( i == 3 ) {
        gradientMatrix.at(1, 1) = 2.0 * this->Cnn;
        gradientMatrix.at(2, 2) = 2.0 * this->Css;
    }
}

void
Masonry02 :: computeReducedSKGradientMatrix(FloatMatrix &gradientMatrix,  int i, GaussPoint *gp, const FloatArray &stressVector,
                                            const FloatArray &strainSpaceHardeningVariables) const
{
    // computes mixed derivative of load function with respect to stress and hardening variables
    gradientMatrix.resize(2, 3);
    gradientMatrix.zero();

    if ( i == 2 ) {
#if 0
        if ( 0 ) {
            // double help = this->gfI*this->c0/(this->gfII*this->ft0);
            double k2 = strainSpaceHardeningVariables.at(2);
            double c =   this->c0 * exp( ( -1.0 ) * this->c0 * k2 / this->gfII );


            //double tanfi=tanfi0+(tanfir-tanfi0)*(c0-c)/c0;
            //double nx = tanfi; double ny=sgn (stressVector.at(2));

            gradientMatrix.at(1, 2) = ( -1.0 ) * ( tanfir - tanfi0 ) * c * ( -1.0 ) / this->gfII;
        }
#endif
        /*
         * // test fan region
         * if (((nx*(stressVector.at(1)-c) + ny*stressVector.at(2)) > 0.0) &&
         *  ((nx*(stressVector.at(1)-c) - ny*stressVector.at(2)) > 0.0)) {
         * // fan region active
         * gradientMatrix.at(1,2) = (-2.0)*c*(-1.0)*this->c0/this->gfII;
         * } else {
         * gradientMatrix.at(1,2) = (-1.0)*(tanfir-tanfi0)*c*(-1.0)/this->gfII;
         * }
         */
    }
}


MaterialStatus *
Masonry02 :: CreateStatus(GaussPoint *gp) const
{
    return new MPlasticMaterial2Status(gp, this->giveSizeOfReducedHardeningVarsVector(gp));
    /*
     * // introduce initial pre-softening (using strainSpaceHardeningVarsVector) to
     * // avoid problems with undefined hardening moduli.
     * double factor;
     * factor = log(0.99);
     * FloatArray strainSpaceHardeningVarsVector(2);
     * strainSpaceHardeningVarsVector.at(1) = gfI*factor/ft0;
     * factor = log(0.95);
     * strainSpaceHardeningVarsVector.at(2) = gfII*factor/c0;
     * status->letTempStrainSpaceHardeningVarsVectorBe(strainSpaceHardeningVarsVector);
     * status->letStrainSpaceHardeningVarsVectorBe(strainSpaceHardeningVarsVector);
     */
}

void
Masonry02 :: giveStiffnessMatrix(FloatMatrix &answer,
                                 MatResponseMode rMode,
                                 GaussPoint *gp, TimeStep *tStep) const
//
// Returns characteristic material stiffness matrix of the receiver
//
{
    MaterialMode mMode = gp->giveMaterialMode();
    switch ( mMode ) {
    case _2dInterface:
        give2dInterfaceMaterialStiffnessMatrix(answer, rMode, gp, tStep);
        break;
    default:
        MPlasticMaterial2 :: giveStiffnessMatrix(answer, rMode, gp, tStep);
    }
}


void
Masonry02 :: give2dInterfaceMaterialStiffnessMatrix(FloatMatrix &answer, MatResponseMode mode,
                                                    GaussPoint *gp, TimeStep *tStep) const
{
    if ( mode == TangentStiffness ) {
        if ( rmType == mpm_ClosestPoint ) {
            answer = this->giveConsistentStiffnessMatrix(mode, gp, tStep);
        } else {
            answer = this->giveElastoPlasticStiffnessMatrix(mode, gp, tStep);
        }
    } else {
        this->computeReducedElasticModuli(answer, gp, tStep);
    }
}


void
Masonry02 :: computeReducedElasticModuli(FloatMatrix &answer,
                                         GaussPoint *gp,
                                         TimeStep *tStep) const
{  /* Returns elastic moduli in reduced stress-strain space*/
    MaterialMode mode = gp->giveMaterialMode();
    if ( mode == _2dInterface ) {
        answer.resize(2, 2);
        answer.at(1, 1) = kn;
        answer.at(2, 2) = ks;
        answer.at(1, 2) = answer.at(2, 1) = 0.0;
    } else {
        this->linearElasticMaterial->giveStiffnessMatrix(answer, ElasticStiffness, gp, tStep);
    }
}

/*
 * #define sic 1./3.
 * #define spc 1.0
 * #define smc 0.5
 * #define src 1./7.
 *
 **#define kp 0.09
 **#define km 0.49
 **#define kr 1.e6
 */

double
Masonry02 :: computeF3HardeningLaw(double k) const
{
    // ideal case

    if ( ( k > 0. ) && ( k < kp ) ) {
        return ( sic - spc ) * k * k / kp / kp - 2. * ( sic - spc ) * kp * k / kp / kp + sic;
        //return sic+(spc-sic)*sqrt(2.*k/kp-k*k/kp/kp);
    } else if ( ( k >= kp ) && ( k < km ) ) {
        double h = ( k - kp ) / ( km - kp );
        return spc + ( smc - spc ) * h * h;
    } else if ( k >= km ) {
        double m = 2.0 * ( smc - spc ) / ( km - kp );
        return src + ( smc - src ) * exp( m * ( k - km ) / ( ( smc - src ) ) );
    } else if ( k <= 0. ) {
        return sic;
    }


    if ( ( k >= 0. ) && ( k < kp ) ) {
        return ( sic - spc ) * k * k / kp / kp - 2. * ( sic - spc ) * kp * k / kp / kp + sic;
        //return sic+(spc-sic)*sqrt(2.*k/kp-k*k/kp/kp);
    } else if ( ( k >= kp ) && ( k < km ) ) {
        double h = ( k - kp ) / ( km - kp );
        return spc + ( smc - spc ) * h * h;
    } else if ( k >= km ) {
        double m = 2.0 * ( smc - spc ) / ( km - kp );
        return src + ( smc - src ) * exp( m * ( k - km ) / ( ( smc - src ) ) );
    } else if ( k < 0. ) {
        double grad = -2.0 * ( ( sic - spc ) / kp / kp ) * kp;
        return max(sic + grad * k, 0.);
    } else if ( ( k < 0. ) && ( k > -kp ) ) {
        // artificial prolongation to negative values
        return ( sic - spc ) * k * k / kp / kp - 2. * ( sic - spc ) * kp * k / kp / kp + sic;
        //return sic-(spc-sic)*sqrt(-2.*k/kp-k*k/kp/kp);
    } else if ( k <= -kp ) {
        // artificial prolongation to negative values
        return sic - ( spc - sic );
    }

    return 0.0;
}

double
Masonry02 :: computeF3HardeningGradient(double k) const
{
    // use secant stiffness
    if ( k < 0. ) {
        return 0.;
    } else if ( k == 0. ) {
        return 2. * k * ( sic - spc ) / kp / kp - 2.0 * ( ( sic - spc ) / kp / kp ) * kp;
    } else if ( k < km ) {
        double st = ( computeF3HardeningLaw(k) - sic );
        return st / k;
    } else {
        double st = ( computeF3HardeningLaw(k) - src );
        return st / k;
    }
    // Unreachable code - commeted out
#if 0
    /*
     * if (k==0.) {
     * //return 1.e20;
     * }
     */

    // ideal case
    if ( k <= 0. ) {
        return 0.0;
    } else if ( ( k >= 0. ) && ( k < kp ) ) {
        return 2. * k * ( sic - spc ) / kp / kp - 2.0 * ( ( sic - spc ) / kp / kp ) * kp;
        //return (spc-sic)*2.0*(1./kp-k/kp/kp)/(2.0*sqrt(2.*k/kp-k*k/kp/kp));
    } else if ( ( k >= kp ) && ( k < km ) ) {
        double h = ( k - kp ) / ( km - kp );
        return ( smc - spc ) * 2.0 * h / ( km - kp );
    } else if ( k >= km ) {
        double m = 2.0 * ( smc - spc ) / ( km - kp );
        double result = ( smc - src ) * exp( m * ( k - km ) / ( ( smc - src ) ) ) * m / ( ( smc - src ) );
        return result;
        //if (res > 1.e-5) return 1.e-5;
        //else return res;
    }



#endif

#if 0

    if ( k < 0. ) {
        double grad = -2.0 * ( ( sic - spc ) / kp / kp ) * kp;
        if ( ( sic + grad * k ) <= 0. ) {
            return 0.;
        } else {
            return grad;
        }
    } else if ( ( k >= 0. ) && ( k < kp ) ) {
        return 2. * k * ( sic - spc ) / kp / kp - 2.0 * ( ( sic - spc ) / kp / kp ) * kp;
        //return (spc-sic)*2.0*(1./kp-k/kp/kp)/(2.0*sqrt(2.*k/kp-k*k/kp/kp));
    } else if ( ( k >= kp ) && ( k < km ) ) {
        double h = ( k - kp ) / ( km - kp );
        return ( smc - spc ) * 2.0 * h / ( km - kp );
    } else if ( k >= km ) {
        double m = 2.0 * ( smc - spc ) / ( km - kp );
        return ( smc - src ) * exp( m * ( k - km ) / ( ( smc - src ) ) ) * m / ( ( smc - src ) );
    } else if ( ( k < 0. ) && ( k > -kp ) ) {
        // artificial prolongation to negative values
        return 2. * k * ( sic - spc ) / kp / kp - 2.0 * ( ( sic - spc ) / kp / kp ) * kp;
        //return (-1.0)*(spc-sic)*(-2.0)*(1./kp+k/kp/kp)/(2.0*sqrt(-2.*k/kp-k*k/kp/kp));
    } else if ( k <= -kp ) {
        // artificial prolongation to negative values
        return 0.0;
    }

    return 0.0;

#endif
}
} // end namespace oofem
