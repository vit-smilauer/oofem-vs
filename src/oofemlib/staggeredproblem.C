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

#include "staggeredproblem.h"
#include "oofemcfg.h"
#include "engngm.h"
#include "timestep.h"
#include "function.h"
#include "metastep.h"
#include "exportmodulemanager.h"
#include "mathfem.h"
#include "oofemtxtdatareader.h"
#include "util.h"
#include "verbose.h"
#include "classfactory.h"
#include "domain.h"

#include <stdlib.h>

#ifdef __OOFEG
 #include "oofeggraphiccontext.h"
#endif

namespace oofem {
REGISTER_EngngModel(StaggeredProblem);

StaggeredProblem :: StaggeredProblem(int i, EngngModel *_master) : EngngModel(i, _master),
    adaptiveStepLength(false),
    minStepLength(0.),
    maxStepLength(0.),
    reqIterations(0.),
    adaptiveStepSince(0.),
    endOfTimeOfInterest(0.),
    prevStepLength(0.),
    currentStepLength(0.)
{
    ndomains = 1; // domain is needed to store the time step function

    dtFunction = 0;
    stepMultiplier = 1.;
    timeDefinedByProb = 0;
}

StaggeredProblem :: ~StaggeredProblem()
{}

///////////
int
StaggeredProblem :: instanciateYourself(DataReader &dr, InputRecord &ir, const char *dataOutputFileName, const char *desc)
{
    int result;
    result = EngngModel :: instanciateYourself(dr, ir, dataOutputFileName, desc);
    ir.finish();
    // instanciate slave problems
    result &= this->instanciateSlaveProblems();
    return result;
}

int
StaggeredProblem :: instanciateDefaultMetaStep(InputRecord &ir)
{
    if ( timeDefinedByProb ) {
        /* just set a nonzero number of steps;
         * needed for instanciateDefaultMetaStep to pass; overall has no effect as time stepping is deteremined by slave
         */
        this->numberOfSteps = 1;
    }
    EngngModel :: instanciateDefaultMetaStep(ir);
    //there are no slave problems initiated so far, the overall metaStep will defined in a slave problem instantiation
    return 1;
}

int
StaggeredProblem :: instanciateSlaveProblems()
{
    //first instantiate master problem if defined
    //EngngModel *timeDefProb = NULL;
    emodelList.resize( inputStreamNames.size() );
    if ( timeDefinedByProb ) {
        OOFEMTXTDataReader dr(inputStreamNames [ timeDefinedByProb - 1 ]);
        std :: unique_ptr< EngngModel >prob( InstanciateProblem(dr, this->pMode, this->contextOutputMode, this) );
        //timeDefProb = prob.get();
        emodelList [ timeDefinedByProb - 1 ] = std :: move(prob);
    }

    for ( int i = 1; i <= ( int ) inputStreamNames.size(); i++ ) {
        if ( i == timeDefinedByProb ) {
            continue;
        }

        OOFEMTXTDataReader dr(inputStreamNames [ i - 1 ]);
        //the slave problem dictating time needs to have attribute master=NULL, other problems point to the dictating slave
        std :: unique_ptr< EngngModel >prob( InstanciateProblem(dr, this->pMode, this->contextOutputMode, this) );
        emodelList [ i - 1 ] = std :: move(prob);
    }

    return 1;
}


void
StaggeredProblem :: initializeFrom(InputRecord &ir)
{
    IR_GIVE_FIELD(ir, numberOfSteps, _IFT_EngngModel_nsteps);
    if ( numberOfSteps <= 0 ) {
        throw ValueInputException(ir, _IFT_EngngModel_nsteps, "nsteps must be > 0");
    }
    if ( ir.hasField(_IFT_StaggeredProblem_deltat) ) {
        EngngModel :: initializeFrom(ir);
        IR_GIVE_FIELD(ir, deltaT, _IFT_StaggeredProblem_deltat);
        dtFunction = 0;
    } else if ( ir.hasField(_IFT_StaggeredProblem_prescribedtimes) ) {
        EngngModel :: initializeFrom(ir);
        IR_GIVE_FIELD(ir, discreteTimes, _IFT_StaggeredProblem_prescribedtimes);
        dtFunction = 0;
    } else if ( ir.hasField(_IFT_StaggeredProblem_dtf) ) {
        IR_GIVE_OPTIONAL_FIELD(ir, dtFunction, _IFT_StaggeredProblem_dtf);
    } else {
        IR_GIVE_FIELD(ir, timeDefinedByProb, _IFT_StaggeredProblem_timeDefinedByProb);
    }

    if ( ir.hasField(_IFT_StaggeredProblem_adaptiveStepLength) ) {
        adaptiveStepLength = true;
        this->minStepLength = 0.;
        IR_GIVE_OPTIONAL_FIELD(ir, minStepLength, _IFT_StaggeredProblem_minsteplength);
        this->maxStepLength = 1.e32;
        IR_GIVE_OPTIONAL_FIELD(ir, maxStepLength, _IFT_StaggeredProblem_maxsteplength);
        this->reqIterations = 1;
        IR_GIVE_OPTIONAL_FIELD(ir, reqIterations, _IFT_StaggeredProblem_reqiterations);
        this->endOfTimeOfInterest = 1.e32;
        IR_GIVE_OPTIONAL_FIELD(ir, endOfTimeOfInterest, _IFT_StaggeredProblem_endoftimeofinterest);
        this->adaptiveStepSince = 0.;
        IR_GIVE_OPTIONAL_FIELD(ir, adaptiveStepSince, _IFT_StaggeredProblem_adaptivestepsince);
    }


    IR_GIVE_OPTIONAL_FIELD(ir, stepMultiplier, _IFT_StaggeredProblem_stepmultiplier);
    if ( stepMultiplier < 0 ) {
        throw ValueInputException(ir, _IFT_StaggeredProblem_stepmultiplier, "stepMultiplier must be > 0");
    }

    //    timeLag = 0.;
    //    IR_GIVE_OPTIONAL_FIELD(ir, timeLag, _IFT_StaggeredProblem_timeLag);

    inputStreamNames.resize(2);
    if ( ir.hasField(_IFT_StaggeredProblem_prob3) ){
        inputStreamNames.resize(3);
    }
    
    IR_GIVE_FIELD(ir, inputStreamNames [ 0 ], _IFT_StaggeredProblem_prob1);
    IR_GIVE_FIELD(ir, inputStreamNames [ 1 ], _IFT_StaggeredProblem_prob2);
    IR_GIVE_OPTIONAL_FIELD(ir, inputStreamNames [ 2 ], _IFT_StaggeredProblem_prob3);
    
    
    renumberFlag = true; // The staggered problem itself should always try to check if the sub-problems needs renumbering.

    coupledModels.resize(3);
    IR_GIVE_OPTIONAL_FIELD(ir, this->coupledModels, _IFT_StaggeredProblem_coupling);


    if ( dtFunction < 1 ) {
        ndomains = 0;
        domainNeqs.clear();
        domainPrescribedNeqs.clear();
        domainList.clear();
    }

    suppressOutput = ir.hasField(_IFT_EngngModel_suppressOutput);

    if ( suppressOutput ) {
        printf("Suppressing output.\n");
    } else {

        if ( ( outputStream = fopen(this->dataOutputFileName.c_str(), "w") ) == NULL ) {
            throw ValueInputException(ir, "None", "can't open output file: " + this->dataOutputFileName);
        }

        fprintf(outputStream, "%s", PRG_HEADER);
        fprintf(outputStream, "\nStarting analysis on: %s\n", ctime(& this->startTime) );
        fprintf(outputStream, "%s\n", simulationDescription.c_str());

#ifdef __MPI_PARALLEL_MODE
        if ( this->isParallel() ) {
            fprintf(outputStream, "Problem rank is %d/%d on %s\n\n", this->rank, this->numProcs, this->processor_name);
        }
#endif
    }
}


void
StaggeredProblem :: updateAttributes(MetaStep *mStep)
{
    auto &ir = mStep->giveAttributesRecord();

    EngngModel :: updateAttributes(mStep);

    // update attributes of slaves
    for ( auto &emodel: emodelList ) {
        emodel->updateAttributes(mStep);
    }

    if ( !timeDefinedByProb ) {
        if ( ir.hasField(_IFT_StaggeredProblem_deltat) ) {
            IR_GIVE_FIELD(ir, deltaT, _IFT_StaggeredProblem_deltat);
            IR_GIVE_OPTIONAL_FIELD(ir, dtFunction, _IFT_StaggeredProblem_dtf);
            IR_GIVE_OPTIONAL_FIELD(ir, stepMultiplier, _IFT_StaggeredProblem_stepmultiplier);
            if ( stepMultiplier < 0 ) {
                OOFEM_ERROR("stepMultiplier must be > 0")
            }
        } else if ( ir.hasField(_IFT_StaggeredProblem_prescribedtimes) ) {
            IR_GIVE_FIELD(ir, discreteTimes, _IFT_StaggeredProblem_prescribedtimes);
        }
    }
}

Function *StaggeredProblem :: giveDtFunction()
// Returns the load-time function of the receiver.
{
    if ( !dtFunction ) {
        return NULL;
    }

    return giveDomain(1)->giveFunction(dtFunction);
}

double
StaggeredProblem :: giveDeltaT(int n)
{
    if ( giveDtFunction() ) {
        return giveDtFunction()->evaluateAtTime(n);
    }

    //in the first step the time increment is taken as the initial, user-specified value
    if ( stepMultiplier != 1 && currentStep ) {
        if ( currentStep->giveNumber() >= 2 ) {
            return ( currentStep->giveTargetTime() * ( stepMultiplier ) );
        }
    }

    if ( discreteTimes.giveSize() > 0 ) {
        return this->giveDiscreteTime(n) - this->giveDiscreteTime(n - 1);
    }

    if ( adaptiveStepLength ) {
        EngngModel *sp;
        int nite = 1;
        double adjustedDeltaT = deltaT;

        if ( currentStep != NULL ) {
            if ( currentStep->giveNumber() != 0 ) {
                // return prescribed deltaT for times until time = adaptiveStepSince
                // can be used for consecutive force loading applied in a specified number of steps
                if ( !( currentStep->giveTargetTime() > this->adaptiveStepSince ) ) {
                    return adjustedDeltaT;
                }

                for ( int i = 1; i <= this->giveNumberOfSlaveProblems(); i++ ) {
                    sp = this->giveSlaveProblem(i);
                    nite = max(sp->giveCurrentNumberOfIterations(), nite);
                }

                if ( nite > reqIterations ) {
                    adjustedDeltaT =  this->prevStepLength * reqIterations / nite;
                } else {
                    adjustedDeltaT  =  this->prevStepLength * sqrt( sqrt( ( double ) reqIterations / ( double ) nite ) );
                }

                if ( adjustedDeltaT > maxStepLength ) {
                    adjustedDeltaT = maxStepLength;
                }

                if ( adjustedDeltaT < minStepLength ) {
                    adjustedDeltaT = minStepLength;
                }
            }
        }

        this->currentStepLength = adjustedDeltaT;

        return adjustedDeltaT;
    }

    return deltaT;
}

double
StaggeredProblem :: giveDiscreteTime(int iStep)
{
    if ( ( iStep > 0 ) && ( iStep <= discreteTimes.giveSize() ) ) {
        return ( discreteTimes.at(iStep) );
    }

    if ( ( iStep == 0 ) && ( iStep <= discreteTimes.giveSize() ) ) {
        return ( 0.0 );
    }

    OOFEM_ERROR("invalid iStep");
}

TimeStep *
StaggeredProblem :: giveCurrentStep(bool force)
{
    if ( timeDefinedByProb ) {
        return emodelList [ timeDefinedByProb - 1 ].get()->giveCurrentStep(true);
    } else {
        return EngngModel :: giveCurrentStep();
    }
}

TimeStep *
StaggeredProblem :: givePreviousStep(bool force)
{
    if ( timeDefinedByProb ) {
        return emodelList [ timeDefinedByProb - 1 ].get()->givePreviousStep(true);
    } else {
        return EngngModel :: givePreviousStep();
    }
}

TimeStep *
StaggeredProblem :: giveSolutionStepWhenIcApply(bool force)
{
    if ( timeDefinedByProb ) {
        return emodelList [ timeDefinedByProb - 1 ].get()->giveSolutionStepWhenIcApply(true);
    } else {
        if ( !stepWhenIcApply ) {
            int nFirst = giveNumberOfFirstStep();
            //stepWhenIcApply = std::make_unique<TimeStep>(giveNumberOfTimeStepWhenIcApply(), this, 0, -giveDeltaT(nFirst), giveDeltaT(nFirst), 0); //previous version for [-dt, 0]
            stepWhenIcApply = std::make_unique<TimeStep>(giveNumberOfTimeStepWhenIcApply(), this, 0, 0., giveDeltaT(nFirst), 0); //now go from [0, dt]
        }

        return stepWhenIcApply.get();
    }
}


EngngModel *
StaggeredProblem :: giveTimeControl(){
    if ( !timeDefinedByProb ) {
        return this;
    } else { //time dictated by slave problem
        return this->giveSlaveProblem(timeDefinedByProb);
    }
}


int
StaggeredProblem :: giveNumberOfFirstStep(bool force)
{
    if ( timeDefinedByProb && !force) {
        return emodelList [ timeDefinedByProb - 1 ].get()->giveNumberOfFirstStep(true);
    } else {
        return EngngModel :: giveNumberOfFirstStep(force);
    }
}


TimeStep *
StaggeredProblem :: giveNextStep()
{
    int istep = this->giveNumberOfFirstStep();
    double totalTime = 0;
    StateCounterType counter = 1;

    if ( !currentStep ) {
        // first step -> generate initial step
        currentStep = std::make_unique<TimeStep>( *giveSolutionStepWhenIcApply() );
    }

    double dt = this->giveDeltaT(currentStep->giveNumber()+1);
    istep =  currentStep->giveNumber() + 1;
    totalTime = currentStep->giveTargetTime() + this->giveDeltaT(istep);
    counter = currentStep->giveSolutionStateCounter() + 1;
    previousStep = std :: move(currentStep);
    currentStep = std::make_unique<TimeStep>(*previousStep, dt);

    if ( ( totalTime >= this->endOfTimeOfInterest ) && this->adaptiveStepLength ) {
        totalTime = this->endOfTimeOfInterest;
        OOFEM_LOG_INFO("\n==================================================================\n");
        OOFEM_LOG_INFO( "\nAdjusting time step length to: %lf \n\n", totalTime - previousStep->giveTargetTime() );
        currentStep = std::make_unique<TimeStep>(istep, this, 1, totalTime, totalTime - previousStep->giveTargetTime(), counter);
        this->numberOfSteps = istep;
    } else {
        if ( this->adaptiveStepLength ) {
            OOFEM_LOG_INFO("\n==================================================================\n");
            OOFEM_LOG_INFO( "\nAdjusting time step length to: %lf \n\n", totalTime - previousStep->giveTargetTime() );
        }
        currentStep = std::make_unique<TimeStep>(istep, this, 1, totalTime, totalTime - previousStep->giveTargetTime(), counter);
    }

    // time and dt variables are set eq to 0 for statics - has no meaning
    return currentStep.get();
}


void
StaggeredProblem :: solveYourself()
{
    EngngModel *sp;
    sp = giveTimeControl();

    int smstep = 1, sjstep = 1;
    this->timer.startTimer(EngngModelTimer :: EMTT_AnalysisTimer);

    if ( sp->giveCurrentStep() ) {
        smstep = sp->giveCurrentStep()->giveMetaStepNumber();
        sjstep = sp->giveMetaStep(smstep)->giveStepRelativeNumber( sp->giveCurrentStep()->giveNumber() ) + 1;
    }

    for ( int imstep = smstep; imstep <= sp->giveNumberOfMetaSteps(); imstep++ ) { //loop over meta steps
        MetaStep *activeMStep = sp->giveMetaStep(imstep);
        // update state according to new meta step in all slaves
        this->initMetaStepAttributes(activeMStep);

        int nTimeSteps = activeMStep->giveNumberOfSteps();
        for ( int jstep = sjstep; jstep <= nTimeSteps; jstep++ ) { //loop over time steps
            this->timer.startTimer(EngngModelTimer :: EMTT_SolutionStepTimer);
            this->timer.initTimer(EngngModelTimer :: EMTT_NetComputationalStepTimer);
            sp->preInitializeNextStep();
            sp->giveNextStep();

            // renumber equations if necessary. Ensure to call forceEquationNumbering() for staggered problems
            if ( this->requiresEquationRenumbering( sp->giveCurrentStep() ) ) {
                this->forceEquationNumbering();
            }

            this->initializeYourself( sp->giveCurrentStep() );
            this->solveYourselfAt( sp->giveCurrentStep() );
            this->updateYourself( sp->giveCurrentStep() );
            this->timer.stopTimer(EngngModelTimer :: EMTT_SolutionStepTimer);
            double _steptime = this->timer.getUtime(EngngModelTimer :: EMTT_SolutionStepTimer);
            sp->giveCurrentStep()->solutionTime = _steptime;
            
            this->terminate( sp->giveCurrentStep() );

            OOFEM_LOG_INFO("EngngModel info: user time consumed by solution step %d: %.2fs\n",
                           sp->giveCurrentStep()->giveNumber(), _steptime);

            if(!suppressOutput) {
            	fprintf(this->giveOutputStream(), "\nUser time consumed by solution step %d: %.3f [s]\n\n",
                        sp->giveCurrentStep()->giveNumber(), _steptime);
            }

#ifdef __MPI_PARALLEL_MODE
            if ( loadBalancingFlag ) {
                this->balanceLoad( sp->giveCurrentStep() );
            }

#endif

            if ( ( sp->giveCurrentStep()->giveTargetTime() >= this->endOfTimeOfInterest ) && this->adaptiveStepLength ) {
                break;
            }
        }
    }
}

void
StaggeredProblem :: solveYourselfAt(TimeStep *tStep)
{
#ifdef VERBOSE
    OOFEM_LOG_RELEVANT("Solving [step number %5d, time %e]\n", tStep->giveNumber(), tStep->giveTargetTime());
#endif
    for ( auto &emodel: emodelList ) {
        emodel->solveYourselfAt(tStep);
    }

    tStep->incrementStateCounter();
}

int
StaggeredProblem :: forceEquationNumbering()
{
    int neqs = 0;
    for ( auto &emodel: emodelList ) {
        // renumber equations if necessary
        if ( emodel->requiresEquationRenumbering( emodel->giveCurrentStep() ) ) {
            neqs += emodel->forceEquationNumbering();
        }
    }

    return neqs;
}

void
StaggeredProblem :: updateYourself(TimeStep *tStep)
{
    if ( adaptiveStepLength ) {
        this->prevStepLength = this->currentStepLength;
    }

    for ( auto &emodel: emodelList ) {
        emodel->updateYourself(tStep);
    }

    EngngModel :: updateYourself(tStep);
}

void
StaggeredProblem :: terminate(TimeStep *tStep)
{
    for ( auto &emodel: emodelList ) {
        emodel->terminate(tStep);
    }
}

void
StaggeredProblem :: doStepOutput(TimeStep *tStep)
{
    for ( auto &emodel: emodelList ) {
        emodel->giveExportModuleManager()->doOutput(tStep);
    }
}


void
StaggeredProblem :: printOutputAt(FILE *file, TimeStep *tStep)
{
    // Subproblems handle the output themselves.
}


void
StaggeredProblem :: saveContext(DataStream &stream, ContextMode mode)
{
    EngngModel :: saveContext(stream, mode);
    for ( auto &emodel: emodelList ) {
        emodel->saveContext(stream, mode);
    }
}


void
StaggeredProblem :: restoreContext(DataStream &stream, ContextMode mode)
{
    EngngModel :: restoreContext(stream, mode);
    for ( auto &emodel: this->emodelList ) {
        emodel->restoreContext(stream, mode);
    }
}


EngngModel *
StaggeredProblem :: giveSlaveProblem(int i)
{
    if ( ( i > 0 ) && ( i <= this->giveNumberOfSlaveProblems() ) ) {
        return this->emodelList [ i - 1 ].get();
    } else {
        OOFEM_ERROR("Undefined problem");
    }

    // return NULL;
}


int
StaggeredProblem :: checkProblemConsistency()
{
    // check internal consistency
    // if success returns nonzero
    int result = 1;
    for ( auto &emodel: emodelList ) {
        result &= emodel->checkProblemConsistency();
    }

#  ifdef VERBOSE
    if ( result ) {
        OOFEM_LOG_DEBUG("Consistency check:  OK\n");
    } else {
        VERBOSE_PRINTS("Consistency check", "failed")
        exit(1);
    }

#  endif

    return result;
}

void
StaggeredProblem :: updateDomainLinks()
{
    for ( auto &emodel: emodelList ) {
        emodel->updateDomainLinks();
    }
}

void
StaggeredProblem :: setRenumberFlag()
{
    for ( auto &emodel: emodelList ) {
        emodel->setRenumberFlag();
    }
}

#ifdef __OOFEG
void StaggeredProblem :: drawYourself(oofegGraphicContext &gc)
{
    int ap = gc.getActiveProblemIndx();
    if ( ( ap > 0 ) && ( ap <= giveNumberOfSlaveProblems() ) ) {
        this->giveSlaveProblem(ap)->drawYourself(gc);
    }
}

void StaggeredProblem :: drawElements(oofegGraphicContext &gc)
{
    int ap = gc.getActiveProblemIndx();
    if ( ( ap > 0 ) && ( ap <= giveNumberOfSlaveProblems() ) ) {
        this->giveSlaveProblem(ap)->drawElements(gc);
    }
}

void StaggeredProblem :: drawNodes(oofegGraphicContext &gc)
{
    int ap = gc.getActiveProblemIndx();
    if ( ( ap > 0 ) && ( ap <= giveNumberOfSlaveProblems() ) ) {
        this->giveSlaveProblem(ap)->drawNodes(gc);
    }
}
#endif
} // end namespace oofem
