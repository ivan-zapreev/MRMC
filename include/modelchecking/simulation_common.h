/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: simulation_common.h,v $
*	$Revision: 1.25 $
*	$Date: 2007/10/14 16:31:55 $
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

#include "macro.h"

#include "sample_vec.h"
#include "sparse.h"
#include "bitset.h"

#ifndef SIMULATION_COMMON_H

#define SIMULATION_COMMON_H
	
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
	extern inline BOOL doesComparisonHold( const double probability, const int comparator, const double prob_bound );
	
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
						const int initial_state, const BOOL isSimOneInitState );
#endif
