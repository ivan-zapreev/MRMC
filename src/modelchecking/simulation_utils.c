/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: simulation_utils.c,v $
*	$Revision: 1.14 $
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
*	here we intend to define the utility functions and data structures.
*/

#include "simulation_common.h"

#include "parser_to_tree.h"
#include "rand_num_generator.h"

/****************************************************************************/
/**************CHECK THE CONF. INT. AGAINST THE PROB. CONSTRAINT*************/
/****************************************************************************/

static inline TV_LOGIC checkBoundVSConfIntHelper(BOOL theTRUECond, BOOL theFALSECond){
	TV_LOGIC tvl_result = TVL_NN;
	if( theTRUECond ){
		tvl_result = TVL_TT;
	} else {
		if( theFALSECond ){
			tvl_result = TVL_FF;
		}
	}
	return tvl_result;
}

/**
* This is an implementation of the checkBoundVSConfInt function from the PhD thesis
* of Ivan S. Zapreev, see Chapter 6.
* @param comparator the comparator, one of: 
*	COMPARATOR_SF_GREATER, COMPARATOR_SF_GREATER_OR_EQUAL,
*	COMPARATOR_SF_LESS, COMPARATOR_SF_LESS_OR_EQUAL
* @param prob_bound the probability bound
* @param ciLeftBorder the left border of the conf. int.
* @param ciRightBorder the right border of the conf. int.
* @return returns one of: TVL_TT, TVL_FF, TVL_NN
*/
TV_LOGIC checkBoundVSConfInt( const int comparator, const double prob_bound,
				const double ciLeftBorder, const double ciRightBorder ){
	TV_LOGIC tvl_result = TVL_NN;
	
	//First check if it is a valid conf. int., i.e. ciLeftBorder <= ciRightBorder
	if( ciLeftBorder <= ciRightBorder ){
		//Distinguish the cases based on the comparator type
		switch( comparator ){
			case COMPARATOR_SF_LESS_OR_EQUAL:
				tvl_result = checkBoundVSConfIntHelper( prob_bound >= ciRightBorder,
									prob_bound < ciLeftBorder );
				break;
			case COMPARATOR_SF_LESS:
				tvl_result = checkBoundVSConfIntHelper( prob_bound > ciRightBorder,
									prob_bound <= ciLeftBorder );
				break;
			case COMPARATOR_SF_GREATER_OR_EQUAL:
				tvl_result = checkBoundVSConfIntHelper( prob_bound <= ciLeftBorder,
									prob_bound > ciRightBorder );
				break;
			case COMPARATOR_SF_GREATER:
				tvl_result = checkBoundVSConfIntHelper( prob_bound < ciLeftBorder,
									prob_bound >= ciRightBorder );
				break;
			default:
				printf("ERROR: An unexpected comparator type: %d.\n", comparator);
				exit(1);
		}
	}
	
	return tvl_result;
}

/****************************************************************************/
/*********CHECK THE TRANSIENT PHASE IN SIMULATIONS FOR UNBOUNDED UNTIL*******/
/****************************************************************************/

/**
* This is an implementation of the InTransPhase function from the PhD thesis
* of Ivan S. Zapreev, see Chapter 6. The function tests if we are in the transient phase.
* @param pSampleVecUntilOne the first sample
* @return TRUE if with the convergence confidence of pSampleVecUntilOne is
*		in the transient phase, otherwise FALSE
*/
BOOL isInTransPhase( PTSampleVecUntil pSampleVecUntilOne ){
	BOOL isTransientPhase = TRUE;
	IF_SAFETY( pSampleVecUntilOne != NULL )
		//If the simulation depth is zero then we are clearly in the transient phase
		//Since we do not expect to do simulation for the absorbing states
		//The states at the simulation depth 0 are all transient
		if( pSampleVecUntilOne->curr_simulation_depth > 0 ){
			IF_SAFETY( pSampleVecUntilOne->conv_confidence_zeta >= 1.0 )
				//Define these constants just for the brevity.
				const double cc_zeta = pSampleVecUntilOne->conv_confidence_zeta;
				const double sample_size = (double) pSampleVecUntilOne->curr_sample_size;
				
				//Compute the bound for the number of trans states in the sample
				const double ts_bound = cc_zeta * sample_size / ( cc_zeta + sample_size - 1.0);
				
				//Check if we are in the transient phase or not
				if( pSampleVecUntilOne->sum_trans  <= ts_bound ){
					isTransientPhase = FALSE;
				}
			ELSE_SAFETY
				printf("ERROR: The convergence-confidence zeta is < 1.0.\n");
				exit(1);
			ENDIF_SAFETY
		}
	ELSE_SAFETY
		printf("ERROR: One of the sample pointers in the convergence test is NULL.\n");
		exit(1);
	ENDIF_SAFETY
	
	return isTransientPhase;
}

/****************************************************************************/
/**************THE INCREASE FUNCTION WITH THE STEP AND MAX BOUND*************/
/****************************************************************************/

/**
* This method does the following:
* 1. (* pTarget) += inc_step;
* 2. if (* pTarget) > max_val then
*	(* pTarget) = max_val;
* @param pTarget the pointer to the variable that will be increased.
* @param inc_step the increment step
* @param max_val the maximum value
*/
inline void increment( int * pTarget, const int inc_step, const int max_val ){
	IF_SAFETY( pTarget != NULL )
		(* pTarget) += inc_step;
		if( (* pTarget) > max_val ){
			(* pTarget) = max_val;
		}
	ELSE_SAFETY
		printf("ERROR: Trying to increment a value defined by a NULL pointer.\n");
		exit(1);
	ENDIF_SAFETY
}

/****************************************************************************/
/**************THE INCREASE FUNCTION WITH THE STEP AND MAX BOUND*************/
/****************************************************************************/

/**
* This method computew the conf. int. borders. for the given sample, i.e.
* we compute and set: (*pCiLeftBorder), (*pCiRightBorder).
* NOTE: The borders that do not fit into the interval R[0,1] are truncated.
* @param conf_zeta the zeta value derived from the confidence
* @param pCiLeftBorder the pointer to the left conf. int. border
* @param pCiRightBorder the pointer to the right conf. int. border
* @param pSampleVecUntil the sample we use to derive the conf int
*/
inline void computeBordersUU( const double conf_zeta, double * pCiLeftBorder,
				 double * pCiRightBorder, const PTSampleVecUntil pSampleVecUntil ){
	IF_SAFETY( pSampleVecUntil != NULL )
		IF_SAFETY( ( pCiLeftBorder != NULL ) && ( pCiRightBorder != NULL ) )
			const double sample_size = (double) pSampleVecUntil->curr_sample_size;
			const double sum_good = (double) pSampleVecUntil->sum_good ;
			const double sum_trans = (double) pSampleVecUntil->sum_trans ;
			
			//Compute the left border, for the good states.
			const double tmp_value_good = sqrt( sum_good * ( sample_size - sum_good ) / ( sample_size - 1.0 ) );
			( * pCiLeftBorder ) = ( sum_good - conf_zeta * tmp_value_good ) / sample_size;
			//Check if we've gotten a negative value
			if( ( * pCiLeftBorder ) < 0.0 ){
				( * pCiLeftBorder ) = 0.0;
			}
			
			//Compute the right border, for the good and transient states.
			const double sum_good_trans = sum_good + sum_trans;
			const double tmp_value_good_trans = sqrt( sum_good_trans * ( sample_size - sum_good_trans ) / ( sample_size - 1.0 ) );
			( * pCiRightBorder ) = ( sum_good_trans + conf_zeta * tmp_value_good_trans ) / sample_size;
			if( ( * pCiRightBorder ) > 1.0 ){
				( * pCiRightBorder ) = 1.0;
			}
		ELSE_SAFETY
			printf("ERROR: The left/right conf. int. border is passed by the NULL pointer.\n");
			exit(1);
		ENDIF_SAFETY
	ELSE_SAFETY
		printf("ERROR: The sample vector is passed by the NULL pointer.\n");
		exit(1);
	ENDIF_SAFETY
}

/****************************************************************************/
/**********THE FUNCTION THAT SIMULATES THE SAMPLE FOR UNBOUNDED UNTIL********/
/****************************************************************************/

//These are the same two values and they indicate that the simulation of new observations
//should statr from the first one and also that all the observation in the sample were processed
#define END_OF_OBSERVATIONS -1
#define FOR_ALL_OBSERVATIONS END_OF_OBSERVATIONS

/**
* This function is supposed to perform "depth_steps_needed" successive simulations
* for every observation after and including the "initial_obs_index"
* (if initial_obs_index != -1). The fields of "pSampleVecUntil", such as
*	sum_good, sum_trans, curr_simulation_depth, pTransStateInd, pObservationsVec.
* are updated correspondingly.
* WARNING: We simulate the states ASSUMING there are no self loops!
* @param pStateSpace the sparse matrix of the embedded DTMC with good ad bad
*			states made absorbing
* @param pSampleVecUntil the sample vector obtained on the previous iteration
* @param initial_obs_index the index of the observation we start simulating from
*				in the array pSampleVecUntil->pObservationsVec
* WARNING: For simulating all observation should be set to -1
* @param depth_steps_needed the simulation depth, i.e. the number of times we
*				simulate each observation in the array pObservationsVec
* @param pGoodStates the bitsets containing all the good absorbing states
* @param pTransientStates the bitsets containing all the transient states
*/
static void simulateUnbUntilSampleDTMC( const sparse * pStateSpace, PTSampleVecUntil pSampleVecUntil,
					const int initial_obs_index, const int depth_steps_needed,
					const bitset * pGoodStates, const bitset *pTransientStates ){
	//Note that the "get_idx_next_non_zero" requires the previous found bitset element index
	//Therefore if we are not searching from the beginning of the bitset we have to use 
	//"initial_obs_index - 1" as the initial index
	int curr_obs_idx = ( initial_obs_index == -1 ? initial_obs_index : initial_obs_index - 1 );
	const bitset * pTransStateInd = pSampleVecUntil->pTransStateInd;
	
	//In case we are not adding new observations but just simulating all further
	if( initial_obs_index == FOR_ALL_OBSERVATIONS ){
		//Update the current simulation depth
		pSampleVecUntil->curr_simulation_depth += depth_steps_needed;
	}
	
	//For all the transient-state observation in the sample
	while( ( curr_obs_idx = get_idx_next_non_zero( pTransStateInd, curr_obs_idx  )) != END_OF_OBSERVATIONS ){
		//Take the current value of the curr_obs_idx's observation
		int current_obs_state = pSampleVecUntil->pObservationsVec[curr_obs_idx];
		int extra_simulation_depth = 0;
		
		//WARNING: DO NOT cast BITSET_BLOCK_TYPE into BOOL because get_bit_val
		//returns an "unsigned int" whereas BOOL is only "short"!!!
		
		//Iterate until we are in the absorbing state or we are on the right depth
		while( ( extra_simulation_depth < depth_steps_needed ) && get_bit_val( pTransientStates, current_obs_state) ){
			//Extract the corresponding row of the sparse matrix
			values pOutgoingTrans = pStateSpace->val[ current_obs_state ];
			
			//WARNING: We simulate the state ASSUMING there are no self loops!
			current_obs_state = generateRandNumberDiscrete( pOutgoingTrans.col, pOutgoingTrans.val,
									pStateSpace->ncols[ current_obs_state ] );
			
			//Update the loop-condition variable
			extra_simulation_depth += 1;
		}
		
		//Update the observation pObservationsVec[curr_obs_idx]
		pSampleVecUntil->pObservationsVec[curr_obs_idx] = current_obs_state;
		
		//Update the sum_good, sum_trans and pTransStateInd fields
		if( ! get_bit_val( pTransientStates, current_obs_state) ){
			pSampleVecUntil->sum_trans -= 1;
			set_bit_val( pTransStateInd, curr_obs_idx , FALSE);
			if( get_bit_val( pGoodStates, current_obs_state) ){
				pSampleVecUntil->sum_good += 1;
			}
		}
	}
}

/**
* This function is an implementation of the "extendSamples" function from the PhD Thesis
* of Ivan S. Zapreev. It either add new observations to the sample and simulates them
* or it increases the simulation depth of every observation in the sample.
* NOTE: This method can increase the sample size and the simulation depth at the same time.
* WARNING: We simulate the states ASSUMING there are no self loops!
* @param pStateSpace the sparse matrix of the embedded DTMC with good ad bad
*			states made absorbing
* @param pSampleVecUntil the sample vector obtained on the previous iteration
* @param new_sample_size the new sample size
* @param new_simulation_depth the new simulation depth
* @param pGoodStates the bitsets containing all the good absorbing states
* @param pTransientStates the bitsets containing all the transient states
*/
void simulateSampleVectorUnbUntilDTMC( const sparse * pStateSpace, PTSampleVecUntil pSampleVecUntil,
					const int new_sample_size, const int new_simulation_depth,
					const bitset * pGoodStates, const bitset * pTransientStates){
	IF_SAFETY( pSampleVecUntil != NULL )
		IF_SAFETY( ( pGoodStates != NULL ) && ( pTransientStates != NULL ) )
			IF_SAFETY( pStateSpace != NULL )
				//If we need to extend the sample size
				if( pSampleVecUntil->curr_sample_size < new_sample_size ){
					const int old_sample_size = pSampleVecUntil->curr_sample_size;
					//Extend the sample size
					extendSampleVectorUntil( pSampleVecUntil, new_sample_size );
					//Do simulations for the newly added observations
					simulateUnbUntilSampleDTMC( pStateSpace, pSampleVecUntil, old_sample_size,
									pSampleVecUntil->curr_simulation_depth,
									pGoodStates, pTransientStates);
				}
				
				//If we need to increase the simulation depth
				const int depth_steps_needed = new_simulation_depth - pSampleVecUntil->curr_simulation_depth;
				if( depth_steps_needed > 0 ){
					//Do simulations for all the observations
					simulateUnbUntilSampleDTMC( pStateSpace, pSampleVecUntil, FOR_ALL_OBSERVATIONS,
									depth_steps_needed, pGoodStates, pTransientStates);
				}
			ELSE_SAFETY
				printf("ERROR: The sparse matrix is NULL.\n");
				exit(1);
			ENDIF_SAFETY
		ELSE_SAFETY
			printf("ERROR: The good/transient states are undefined.\n");
			exit(1);
		ENDIF_SAFETY
	ELSE_SAFETY
		printf("ERROR: The sample that has to be simulated further is NULL.\n");
		exit(1);
	ENDIF_SAFETY
}
