/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: simulation.c,v $
*	$Revision: 1.5 $
*	$Date: 2007/10/14 15:11:58 $
*	$Author: zapreevis $
*
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*
*	Copyright (C) The University of Twente, 2004-2007.
*	Authors: Ivan Zapreev
*
*	This program is free software; you can redistribute it and/or
*	modify it under the terms of the GNU General Public License
*	as published by the Free Software Foundation; either version 2
*	of the License, or (at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program; if not, write to the Free Software
*	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*	Our contacts:
*		Formal Methods and Tools Group, University of Twente,
*		P.O. Box 217, 7500 AE Enschede, The Netherlands,
*		Phone: +31 53 4893767, Fax: +31 53 4893247,
*		E-mail: mrmc@cs.utwente.nl
*
*	Source description: This is a header file for the simulation engine
*	here we intend to define the common functions and data structures.
*/

#include "simulation.h"

#include "bitset.h"
#include "sparse.h"

/****************************************************************************/
/********************MANAGE GENERAL SIMULATION PARAMETERS********************/
/****************************************************************************/

//Defines the mode for model checking the steady-state operator.
//Should be FALSE for the Hybrid mode, otherwise TRUE
static BOOL isSteadyStatePure = FALSE;

/**
* This method should be used to turn on the pure simulation
* for the steady-state (long-run) operator
* @param isOnOff TRUE to turn on the pure simulation mode, FALSE for the hybrid mode
*/
inline void setSimSteadyStateModePure( BOOL isOnOff ){
	isSteadyStatePure = isOnOff;
}

/**
* This method should be used to get the pure-simulation ON/OFF
* (for the steady-state (long-run) operator) indicator in the runtime settings.
* @return TRUE if the pure-simulation is ON, otherwise FALSE.
*/
inline BOOL isSimSteadyStateModePure(){
	return isSteadyStatePure;
}

/**
* This method should be used to get the hybrid-simulation ON/OFF
* (for the steady-state (long-run) operator) indicator in the runtime settings.
* @return TRUE if the hybrid-simulation is ON, otherwise FALSE.
*/
inline BOOL isSimSteadyStateModeHybrid(){
	return ! isSimSteadyStateModePure();
}

//The initial state for the "one initial state" simulation
static int sim_initial_state = 0;

/**
* Sets the initial state that is used for the "one initial state" simulation.
* @param the initial state for doing simulations
*/
inline void setSimInitialState( int initial_state ){
	sim_initial_state = initial_state;
}

/**
* Gets the initial state that is used for the "one initial state" simulation.
* @return the initial state for doing simulations
*/
inline int getSimInitialState(){
	return sim_initial_state;
}

//Defines if one or all states should be considered as initial
//TRUE for one initial state, otherwise FALSE;
static BOOL isOneInitState = FALSE;

/**
* This method sets the type of simulation we have in mind,
* for one initial state only or for all stetes
* @param isOnOff TRUE for just one initial state, FALSE for all states
*/
void setSimOneInitState(BOOL isOnOff){
	isOneInitState = isOnOff;
}

/**
* This method indicates the type of simulation we have in mind,
* for one initial state only or for all stetes.
* @return TRUE for just one initial state, FALSE for all states
*/
BOOL isSimOneInitState(){
	return isOneInitState;
}

//Stores the global confidence level for model
//checking the formula with simulation
//WARNING: Should have DEF_GENERAL_CONFIDENCE >= MIN_GENERAL_CONFIDENCE
static double confidence = DEF_GENERAL_CONFIDENCE;

/**
* Sets the general confidence level in case of simulations.
* @param _confidence the confidence level for the formula
*/
void setSimGeneralConfidence(double _confidence){
	if( ( _confidence >= MIN_GENERAL_CONFIDENCE ) && ( _confidence < 1.0 ) ){
		confidence = _confidence;
	}else{
		printf("WARNING: The confidence level should be >= %e and < 1.\n", MIN_GENERAL_CONFIDENCE);
		printf("WARNING: The set command is ignored.\n");
	}
}

/**
* Gets the general confidence level in case of simulations.
* @return the confidence level for the formula
*/
double getSimGeneralConfidence(){
	return confidence;
}

//Stores the global confidence level for the convergence criteria.
//Is needed for the PURE simulation approach of unbounded until.
//WARNING: Should have DEF_CONVERGENCE_CONFIDENCE >= MIN_CONVERGENCE_CONFIDENCE
static double conv_confidence = DEF_CONVERGENCE_CONFIDENCE;

/**
* Sets the convergence-criteria confidence level in case of simulations.
* Is needed for the PURE simulation approach of unbounded until.
* @param _conv_confidence the convergence-criteria confidence level for the formula
*/
void setSimConvConfidence(double _conv_confidence){
	if( ( _conv_confidence >= MIN_CONVERGENCE_CONFIDENCE ) && ( _conv_confidence < 1.0 ) ){
		conv_confidence = _conv_confidence;
	}else{
		printf("WARNING: The confidence level should be >= %e and < 1.\n", MIN_CONVERGENCE_CONFIDENCE);
		printf("WARNING: The set command is ignored.\n");
	}
}

/**
* Gets the termination-criteria confidence level in case of simulations.
* Is needed for the PURE simulation approach of unbounded until.
* @return the termination-criteria confidence level for the formula
*/
double getSimConvConfidence(){
	return conv_confidence;
}

/****************************************************************************/
/**********************MANAGE THE SAMPLE PARAMETERS**************************/
/****************************************************************************/

//Stores the maximum sample size
static int max_sample_size = DEF_MAX_SAMPLE_SIZE;

//Stores the minimum sample size
static int min_sample_size = DEF_MIN_SAMPLE_SIZE;

//Stores the minimum sample size
static int sample_size_step = DEF_SAMPLE_SIZE_STEP;

/**
* Sets the maximum sample size
* @param _max_sample_size the maximum sample size
*/
inline void setSimMaxSampleSize( int _max_sample_size ){
	if( _max_sample_size >= min_sample_size ){
		max_sample_size = _max_sample_size;
	} else {
		printf("WARNING: The maximum sample size should be >= %d.\n", min_sample_size );
		printf("WARNING: The 'set' command is ignored.\n");
	}
}

/**
* Gets the maximum sample size
* @return the maximum sample size
*/
inline int getSimMaxSampleSize(){
	return max_sample_size;
}

/**
* Sets the minimum sample size
* @param _min_sample_size the minimum sample size
*/
inline void setSimMinSampleSize( int _min_sample_size ){
	if( _min_sample_size >= MIN_MIN_SAMPLE_SIZE && _min_sample_size <= max_sample_size ){
		min_sample_size = _min_sample_size;
	} else {
		printf("WARNING: The minimum sample size should be >= %d and <= %d.\n", MIN_MIN_SAMPLE_SIZE, max_sample_size);
		printf("WARNING: The 'set' command is ignored.\n");
	}
}

/**
* Gets the minimum sample size
* @return the minimum sample size
*/
inline int getSimMinSampleSize(){
	return min_sample_size;
}

/**
* Sets the sample size step
* @param _sample_size_step the sample size step
*/
inline void setSimSampleSizeStep( int _sample_size_step ){
	if( _sample_size_step >= MIN_SAMPLE_SIZE_STEP ){
		sample_size_step = _sample_size_step;
	} else {
		printf("WARNING: The sample-size step should be >= %d.\n", MIN_SAMPLE_SIZE_STEP );
		printf("WARNING: The 'set' command is ignored.\n");
	}
}

/**
* Gets the sample size step
* @return the sample size step
*/
inline int getSimSampleSizeStep(){
	return sample_size_step;
}

/****************************************************************************/
/*******************MANAGE THE SIMULATION DEPTH PARAMETERS*******************/
/*******************IS NEEDED FOR M.C. UNBOUNDED UNTL************************/
/***********************USING PURE SIMULATION APPROACH***********************/
/****************************************************************************/

//Stores the maximum simulation depth
static int max_simulation_depth = DEF_MAX_SIMULATION_DEPTH;

//Stores the minimum simulation depth
static int min_simulation_depth = DEF_MIN_SIMULATION_DEPTH;

//Stores the minimum simulation depth
static int simulation_depth_step = DEF_SIMULATION_DEPTH_STEP;

/**
* Sets the maximum simulation depth
* @param _max_simulation_depth the maximum simulation depth
*/
inline void setSimMaxSimulationDepth( int _max_simulation_depth ){
	if( _max_simulation_depth >= min_simulation_depth ){
		max_simulation_depth = _max_simulation_depth;
	} else {
		printf("WARNING: The maximum simulation depth should be >= %d.\n", min_simulation_depth );
		printf("WARNING: The 'set' command is ignored.\n");
	}
}

/**
* Gets the maximum simulation depth
* @return the maximum simulation depth
*/
inline int getSimMaxSimulationDepth(){
	return max_simulation_depth;
}

/**
* Sets the minimum simulation depth
* @param _min_simulation_depth the minimum simulation depth
*/
inline void setSimMinSimulationDepth( int _min_simulation_depth ){
	if( _min_simulation_depth >= MIN_MIN_SIMULATION_DEPTH && _min_simulation_depth <= max_simulation_depth ){
		min_simulation_depth = _min_simulation_depth;
	} else {
		printf("WARNING: The minimum simulation depth should be >= %d and <= %d.\n", MIN_MIN_SIMULATION_DEPTH, max_simulation_depth );
		printf("WARNING: The 'set' command is ignored.\n");
	}
}

/**
* Gets the minimum simulation depth
* @return the minimum simulation depth
*/
inline int getSimMinSimulationDepth(){
	return min_simulation_depth;
}

/**
* Sets the simulation depth step
* @param _simulation_depth_step the simulation depth step
*/
inline void setSimSimulationDepthStep( int _simulation_depth_step ){
	if( _simulation_depth_step >= MIN_SIMULATION_DEPTH_STEP ){
		simulation_depth_step = _simulation_depth_step;
	} else {
		printf("WARNING: The simulation-depth step should be >= %d.\n", MIN_SIMULATION_DEPTH_STEP );
		printf("WARNING: The 'set' command is ignored.\n");
	}
}

/**
* Gets the simulation depth step
* @return the simulation depth step
*/
inline int getSimSimulationDepthStep(){
	return simulation_depth_step;
}

/****************************************************************************/
/*******************PRINT THE SIMULATION RUNTIME PARAMETERS******************/
/****************************************************************************/

/**
* Print the simulation runtime parameters if the simulation is on.
* Prints nothing otherwise. Also: does not print the RNG for exp.
* if isExpNeeded == FALSE.
* @param isExpNeeded if it is needed to print the info about the exp distrib.
*			random number generator.
*/
inline void printRuntimeSimInfo(BOOL isExpNeeded){
	if( isSimulationOn() ){
		printf(" ---Monte Carlo simulation:\n");
		printf(" Simulation type\t = %s\n", (isSimOneInitState()? "ONE":"ALL") );
		if( isSimOneInitState() ){
			printf(" Sim. initial state\t = %d\n", getSimInitialState()+1 );
		}
		printf(" Sim. steady state\t = %s\n", (isSimSteadyStateModePure() ? "PURE":"HYBRID" ) );
		printf(" Confidence level\t = %e\n", getSimGeneralConfidence() );
		printf(" Max sample size\t = %d\n", getSimMaxSampleSize() );
		printf(" Min sample size\t = %d\n", getSimMinSampleSize() );
		printf(" Sample-size step\t = %d\n", getSimSampleSizeStep() );
		
		//Print the runtime parameters of the random number generator
		printRuntimeRNGInfoDiscrete();
		if( isExpNeeded ){
			printRuntimeRNGInfoExp();
		}
		
		//Print the options for the unbounded-until simulation
		//The should be also used in the steady-state operator simulation
		//Then the pure simulation mode is on.
		printf(" Conv. conf. level\t = %e\n", getSimConvConfidence() );
		printf(" Max simulation depth\t = %d\n", getSimMaxSimulationDepth() );
		printf(" Min simulation depth\t = %d\n", getSimMinSimulationDepth() );
		printf(" Simulation-depth step\t = %d\n", getSimSimulationDepthStep() );
		printf("\n");
	}
}
