/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: simulation_ctmc.c,v $
*	$Revision: 1.8 $
*	$Date: 2007/10/14 16:31:54 $
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
*	here we intend to define the CTMC-related functions and data structures.
*/

#include "simulation_ctmc.h"

#include "simulation_common.h"
#include "transient_common.h"

/****************************************************************************/
/**********************THE FUNCTIONS FOR SIMULATING CTMCs********************/
/****************************************************************************/

/**
* This method sorts out the pAUBitSet and pBadBitSet states and updates the sets 
* pYesBitSet and pNoBitSet with respect to the probability constraint
* "comparator prob_bound".
* @param isSimOneInitState TRUE if we simulate one initial state only
* @param pYesBitSet this bitset is going to be filled with the states which
*			satisfy the formula
* @param pNoBitSet this bitset is going to be filled with the states which
*			do not satisfy the formula
* @param pAUBitSet the states that satisfy Phi U Psi with at least probability 1.0
* @param initial_state the initial state index if isSimOneInitState == TRUE
* @param pBadBitSet the states that satisfy Phi U Psi with at most probability 0.0
* @param comparator the comparator, one of: 
*	COMPARATOR_SF_GREATER, COMPARATOR_SF_GREATER_OR_EQUAL,
*	COMPARATOR_SF_LESS, COMPARATOR_SF_LESS_OR_EQUAL
* @param prob_bound the probability bound
*/
static inline considerNotEAndAStatesUUntil( BOOL isSimOneInitState, bitset * pYesBitSet, bitset * pNoBitSet,
						bitset * pAUBitSet, bitset * pBadBitSet, const int initial_state,
						const int comparator, const double prob_bound ){
	BOOL isAUSetGood = doesComparisonHold( 1.0, comparator, prob_bound );
	BOOL isBadSetGood =  doesComparisonHold( 0.0, comparator, prob_bound );
	
	if( isSimOneInitState ){
		//If we simulate just one initial state than all the other data is not iteresting
		if( get_bit_val( pAUBitSet, initial_state ) ){
			if( isAUSetGood ){
				set_bit_val( pYesBitSet, initial_state, BIT_ON );
			} else {
				set_bit_val( pNoBitSet, initial_state, BIT_ON );
			}
		} else {
			if( get_bit_val( pBadBitSet, initial_state ) ){
				if( isBadSetGood ){
					set_bit_val( pYesBitSet, initial_state, BIT_ON );
				} else {
					set_bit_val( pNoBitSet, initial_state, BIT_ON );
				}
			}
		}
	} else {
		//pAUBitSet are that satisfy Phi U Psi with at least probability 1.0
		if( isAUSetGood ){
			or_result( pAUBitSet, pYesBitSet );
		} else {
			or_result( pAUBitSet, pNoBitSet );
		}
		//pBadBitSet are that satisfy Phi U Psi with at most probability 0.0
		if( isBadSetGood ){
			or_result( pBadBitSet, pYesBitSet );
		} else {
			or_result( pBadBitSet, pNoBitSet );
		}
	}
}

/**
* This function is an implementation of the unboundedUntil function from the PhD Thesis
* of Ivan S. Zapreev. The function does the model checking of unbounded until operator
* on the provided CTMC and uses the runtime-simulation settings from the "simulation.h"
* @param pStateSpace the sparse matrix of the embedded DTMC with good and bad
*			states made absorbing
* @param confidence this is is confidence we should use for simulations.
*			Note that since the formulas can be nested one can not use the
*			overall confidence set in simulation.h
* @param pPhiBitSet the bitsets containing all the good absorbing states
* @param pPsiBitSet the bitsets containing all the transient states
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
extern void modelCheckUnboundedUntilCTMC( const sparse * pStateSpace, const double * pCTMCRowSums,
						const double confidence, const bitset *pPhiBitSet,
						const bitset * pPsiBitSet, bitset ** ppYesBitsetResult,
						bitset ** ppNoBitsetResult, double ** ppProbCILeftBorder,
						double ** ppProbCIRightBorder, int * pResultSize,
						const int comparator, const double prob_bound,
						const int initial_state, const BOOL isSimOneInitState ){
	int *pTransientStateIdx=NULL;
	const int state_space_size = pStateSpace->rows;
	sparse *pAbsCTMCStateSpace = init_matrix( state_space_size, state_space_size );
	
	//0: Sort out the 1.0 and 0.0 prob reachabile states
	bitset * pEUBitSet = get_exist_until( pStateSpace, pPhiBitSet, pPsiBitSet );
	bitset * pAUBitSet = get_always_until( pStateSpace, pPhiBitSet, pPsiBitSet, pEUBitSet );
	
	//NOTE: Using pEUBitSet there is NO NEED to compute BSCCs!
	//Because "pEUBitSet = get_exist_until( pStateSpace, pPhiBitSet, pPsiBitSet )"
	//returns the states from which it is possible to go into Psi states via Phi states
	
	//1: Compute the states that has to be absorbing
	//1.1: At this point pBadBitSet contains states from which
	//psi states are not reachable via phi states
	bitset *pBadBitSet = not( pEUBitSet );
	//1.2: At this point pTransientBitSet contains all absorbing states, i.e.
	//Good and Bad absorbing states (including Phi BSCC states)
	bitset * pTransientBitSet = or( pAUBitSet, pBadBitSet );
	//1.3: At this point pTransientBitSet contains only allowed transient states
	//from which there is a way to go to Psi states via Phi states with prob < 1.0
	not_result( pTransientBitSet );
	
	//print_mtx_sparse(pStateSpace);
	
	//3: Make states absorbing
	pTransientStateIdx = count_set( pTransientBitSet );
	initMatrix( pStateSpace, pAbsCTMCStateSpace, pTransientStateIdx );
	
	//4: Obtain the embedded DTMC (WITHOUT THE SELF LOOPS!!!)
	make_embedded_dtmc_out_of_rate_mtx( pAbsCTMCStateSpace, pCTMCRowSums, pTransientStateIdx );
	
	//print_mtx_sparse(pAbsCTMCStateSpace);
	
	//5: Construct the pGoodStates and pTransientStates sets
	//NOTE: They are already constructed, namely: pAUBitSet and pTransientBitSet
	
	//6: Do simulations
	modelCheckUnboundedUntilCommon( pAbsCTMCStateSpace, confidence, pAUBitSet, pTransientBitSet, ppYesBitsetResult,
					ppNoBitsetResult, ppProbCILeftBorder, ppProbCIRightBorder, pResultSize,
					comparator, prob_bound, initial_state, isSimOneInitState );
	
	//7: Update ppYesBitsetResult and ppNoBitsetResult with pAUBitSet and pBadBitSet states.
	considerNotEAndAStatesUUntil( isSimOneInitState, *ppYesBitsetResult, *ppNoBitsetResult,
					pAUBitSet, pBadBitSet, initial_state, comparator, prob_bound );
	
	//8: Restore the state space
	//NOTE: not required for diagonal elements
	restore_rate_mtx_out_of_embedded_dtmc( pAbsCTMCStateSpace, pCTMCRowSums, pTransientStateIdx );
	cleanMatrix( pAbsCTMCStateSpace, pTransientStateIdx);
	
	//print_mtx_sparse(pStateSpace);
	
	//9: Free the resources
	free( pTransientStateIdx );
	free_bitset( pEUBitSet );
	free_bitset( pAUBitSet );
	free_bitset( pBadBitSet );
	free_bitset( pTransientBitSet );
	free_mtx_sparse( pAbsCTMCStateSpace );
}
