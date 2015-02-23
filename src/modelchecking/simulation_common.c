/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: simulation_common.c,v $
*	$Revision: 1.40 $
*	$Date: 2007/10/29 12:01:10 $
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

#include "simulation_common.h"

#include "parser_to_tree.h"
#include "simulation.h"
#include "simulation_utils.h"
#include "sample_vec.h"
#include "gsl/gsl_cdf.h"

/****************************************************************************/
/********THE COMMON FUNCTION FOR SIMULATING UNBOUNDED UNTIL CTMC/DTMC********/
/****************************************************************************/

/**
* This method checks if "probability comparator prob_bound" holds.
* @param probability the left side of the constraint, i.e. the probability
* @param comparator the comparator, one of: 
*	COMPARATOR_SF_GREATER, COMPARATOR_SF_GREATER_OR_EQUAL,
*	COMPARATOR_SF_LESS, COMPARATOR_SF_LESS_OR_EQUAL
* @param prob_bound the probability bound
* @return TRUE if "probability comparator prob_bound" holds, otherwise FALSE
*/
inline BOOL doesComparisonHold( const double probability, const int comparator, const double prob_bound ){
	switch( comparator ){
		case COMPARATOR_SF_GREATER:
			return ( probability > prob_bound ? TRUE : FALSE );
		case COMPARATOR_SF_GREATER_OR_EQUAL:
			return ( probability >= prob_bound ? TRUE : FALSE );
		case COMPARATOR_SF_LESS:
			return ( probability < prob_bound ? TRUE : FALSE );
		case COMPARATOR_SF_LESS_OR_EQUAL:
			return ( probability <= prob_bound ? TRUE : FALSE );
		default:
			printf("ERROR: An unknown comparator type %d.\n", comparator);
			exit(1);
	}
}

/**
*
* @param initial_state the initial state for simulations
* @param conv_conf_zeta the convergence confidence
* @param gen_conf_zeta the desired confidence of the answer
* @param pStateSpace the sparse matrix of the embedded DTMC with good and bad
*			states made absorbing
* @param pGoodStates the bitsets containing all the good absorbing states
* @param pTransientStates the bitsets containing all the transient states
* @param comparator the comparator, one of: 
*	COMPARATOR_SF_GREATER, COMPARATOR_SF_GREATER_OR_EQUAL,
*	COMPARATOR_SF_LESS, COMPARATOR_SF_LESS_OR_EQUAL
* @param prob_bound the probability bound
* @param isOneStateOnly TRUE if we are going to model check one state only, i.e.
*				pCiLeftBorders and pCiRightBorders are of size 1
*
* NOTE: The parameters below will contain the return values
*
* @param pCiLeftBorders for storing the left conf. int. border
* @param pCiRightBorders for storing the right conf. int. border
* @param pYesBitSet contains curr_state if it satisfies the formula
* @param pNoBitSet contains curr_state if it does not satisfy the formula
*/
static void modelCheckOneStateUUCommon( const int initial_state, const double conv_conf_zeta, const double gen_conf_zeta,
					const sparse* pStateSpace, const bitset * pGoodStates, const bitset * pTransientStates,
					const int comparator, const double prob_bound, double *pCiLeftBorders,
					double *pCiRightBorders, bitset * pYesBitSet, bitset * pNoBitSet,
					BOOL isOneStateOnly ){
	TV_LOGIC mc_result = TVL_NN;
	double leftBorderOne = 0.0, rightBorderOne = 1.0, leftBorderTwo = 0, rightBorderTwo = 1.0;
	
	//Initialize the sample size parameters
	int sample_size = getSimMinSampleSize();
	const int sample_size_step = getSimSampleSizeStep();
	const int max_sample_size = getSimMaxSampleSize();
	
	//Initialize the simulation depth parameters
	int simulation_depth = getSimMinSimulationDepth();
	const int simulation_depth_step = getSimSimulationDepthStep();
	const int max_simulation_depth = getSimMaxSimulationDepth();
	
	//Allocate sample vectors
	PTSampleVecUntil pSampleVecUntilOne = allocateSampleVectorUntil( sample_size, initial_state, conv_conf_zeta );
	
	//Simulate until the minimum simulation depth
	simulateSampleVectorUnbUntilDTMC( pStateSpace, pSampleVecUntilOne, sample_size, simulation_depth,
					pGoodStates, pTransientStates);
	//Perform the main simulation cycle
	do{
		//Choose what to increment 
		if( ( isInTransPhase( pSampleVecUntilOne ) &&
			( simulation_depth < max_simulation_depth ) ) || ( sample_size >= max_sample_size ) ){
			increment( &simulation_depth, simulation_depth_step, max_simulation_depth );
		} else {
			increment( &sample_size, sample_size_step, max_sample_size );
		}
		
		//Simulate the sample
		simulateSampleVectorUnbUntilDTMC( pStateSpace, pSampleVecUntilOne, sample_size, simulation_depth,
						pGoodStates, pTransientStates);
		
		//Compute the Conf Int borders
		computeBordersUU( gen_conf_zeta, &leftBorderOne, &rightBorderOne, pSampleVecUntilOne );
		
		//Check the probability constraint
		//WARNING: We can not do any sort of sample swapping here!
		mc_result = checkBoundVSConfInt( comparator, prob_bound, leftBorderOne, rightBorderOne );
	}while( ( mc_result == TVL_NN ) &&
		( ( sample_size < max_sample_size ) || ( simulation_depth < max_simulation_depth ) ) );
	
	//Update the pCiLeftBorders and pCiRightBorders
	const int arr_index = ( isOneStateOnly ? 0 : initial_state );
	pCiLeftBorders[arr_index] = leftBorderOne;
	pCiRightBorders[arr_index] = rightBorderOne;
	
	//printf("Result: %i, borders are: [%lf, %lf], prob bound %d %lf.\n", mc_result,
	//	pCiLeftBorders[arr_index], pCiRightBorders[arr_index], comparator, prob_bound);
	
	//Update the pYesBitSet or pNoBitSet if we have a definite answer
	switch( mc_result ){
		case TVL_TT:
			set_bit_val( pYesBitSet, initial_state, BIT_ON);
			break;
		case TVL_FF:
			set_bit_val( pNoBitSet, initial_state, BIT_ON);
			break;
	}
	
	//printf("mc_result = %d, sample_size = %d and simulation_depth = %d\n",
	//	mc_result, sample_size, simulation_depth);
	//printSampleVectorUntil(pSampleVecUntilOne);
	//printSampleVectorUntil(pSampleVecUntilTwo);
	
	//Free the sample vectors
	freeSampleVectorUntil( pSampleVecUntilOne );
}

/**
* This function is an implementation of the unboundedUntil function from the PhD Thesis
* of Ivan S. Zapreev. The function does the model checking of unbounded until operator
* on the provided DTMC and uses the runtime-simulation settings from the "simulation.h"
* That is why it has so few parameters.
* WARNING: We simulate the states ASSUMING there are no self loops!
* @param pStateSpace the sparse matrix of the embedded DTMC with good and bad
*			states made absorbing
* @param confidence this is is confidence we should use for simulations.
*			Note that since the formulas can be nested one can not use the
*			overall confidence set in simulation.h
* @param pGoodStates the bitsets containing all the good absorbing states
* @param pTransientStates the bitsets containing all the transient states
* @param comparator the comparator, one of: 
*	COMPARATOR_SF_GREATER, COMPARATOR_SF_GREATER_OR_EQUAL,
*	COMPARATOR_SF_LESS, COMPARATOR_SF_LESS_OR_EQUAL
* @param prob_bound the probability bound
* @param isSimOneInitState TRUE if we simulate one initial state only
* @param initial_state the initial state index if isSimOneInitState == TRUE
*
* NOTE: Below we have pointers to the pointers, these are going to be the return values
*
* @param ppYesBitSet this bitset is going to be filled with the states which
*			satisfy the formula
* @param ppNoBitSet this bitset is going to be filled with the states which
*			do not satisfy the formula
* @param ppCiLeftBorders will contain the left conf. int. borders
* @param ppCiRightBorders will contain the right conf. int. borders
* @param pResultSize the pointer to the size of arrays *ppCiLeftBorders and *ppCiRightBorders
*/
void modelCheckUnboundedUntilCommon( const sparse * pStateSpace, const double confidence, const bitset * pGoodStates,
					const bitset * pTransientStates, bitset ** ppYesBitSet, bitset ** ppNoBitSet,
					double ** ppCiLeftBorders, double ** ppCiRightBorders, int * pResultSize,
					const int comparator, const double prob_bound,
					const int initial_state, const BOOL isSimOneInitState ){
	//Compute the convergence confidence zeta value:
	//The Conv. Confidence. "conf" is given for "P( N(0,1) <= z(conf) ) = conf"
	//And this is exactly what we need: a one side bound for N(0,1)
	const double conv_conf_zeta = gsl_cdf_ugaussian_Pinv( getSimConvConfidence() );
	
	//Compute the general confidence zeta value:
	//NOTE: The confidence we have is used to derive z for the left and the
	//right border BUT the TRUE/FALSE answer is based only on one border of
	//the conf. int., i.e. it is given with at least "confidence" confidence.
	const double gen_conf_zeta = gsl_cdf_ugaussian_Pinv( confidence );
	
	//Allocate the resulting bitsets
	const int state_space_size = pStateSpace->rows;
	*ppYesBitSet = get_new_bitset( state_space_size );
	*ppNoBitSet = get_new_bitset( state_space_size );
	
	if( ( *ppYesBitSet != NULL ) && ( *ppYesBitSet != NULL ) ){
		//Depending on whether there is one initial state only or not.
		if( isSimOneInitState ) {
			
			//Check for the valid initial state
			if( ( initial_state < 0) || ( initial_state >= state_space_size ) ){
				printf("ERROR: An ivalid initial state %d, should be >= %d and <= %d.\n",
					initial_state+1, 1, state_space_size );
				exit(1);
			}
			
			//If it is the transient state
			if( get_bit_val( pTransientStates, initial_state ) ){
				//Allocate the return data
				*pResultSize = 1;
				*ppCiLeftBorders = (double * ) calloc( 1, sizeof(double) );
				*ppCiRightBorders = (double * ) calloc( 1, sizeof(double) );
				//Model check the state
				modelCheckOneStateUUCommon( initial_state, conv_conf_zeta, gen_conf_zeta,
								pStateSpace, pGoodStates, pTransientStates,
								comparator, prob_bound,
								*ppCiLeftBorders, *ppCiRightBorders,
								*ppYesBitSet, *ppNoBitSet, isSimOneInitState );
			}else{
				*pResultSize = 0;
			}
		} else {
			//Allocate the return data
			*pResultSize = state_space_size;
			*ppCiLeftBorders = (double * ) calloc( state_space_size, sizeof(double) );
			*ppCiRightBorders = (double * ) calloc( state_space_size, sizeof(double) );
			
			//Work out all the transient states
			int curr_state = -1;
			while( ( curr_state = get_idx_next_non_zero( pTransientStates, curr_state  )) != -1 ){
				//Model check the state
				modelCheckOneStateUUCommon( curr_state, conv_conf_zeta, gen_conf_zeta,
								pStateSpace, pGoodStates, pTransientStates,
								comparator, prob_bound,
								*ppCiLeftBorders, *ppCiRightBorders,
								*ppYesBitSet, *ppNoBitSet, isSimOneInitState );
			}
		}
	} else {
		printf("ERROR: Failed to allocate data structures for the simulation engine.\n");
		exit(1);
	}
}
