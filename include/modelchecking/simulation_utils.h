/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: simulation_utils.h,v $
*	$Revision: 1.7 $
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

#include "macro.h"

#include "sample_vec.h"
#include "sparse.h"
#include "bitset.h"

#ifndef SIMULATION_UTILS_H

#define SIMULATION_UTILS_H
	
	/****************************************************************************/
	/**************CHECK THE CONF. INT. AGAINST THE PROB. CONSTRAINT*************/
	/****************************************************************************/
	
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
	extern TV_LOGIC checkBoundVSConfInt( const int comparator, const double prob_bound,
						const double ciLeftBorder, const double ciRightBorder );
	
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
	extern BOOL isInTransPhase( PTSampleVecUntil pSampleVecUntilOne );
	
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
	extern inline void increment( int * pTarget, const int inc_step, const int max_val );
	
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
	extern inline void computeBordersUU( const double conf_zeta, double * pCiLeftBorder,
					double * pCiRightBorder, const PTSampleVecUntil pSampleVecUntil );
	
	/****************************************************************************/
	/**********THE FUNCTION THAT SIMULATES THE SAMPLE FOR UNBOUNDED UNTIL********/
	/****************************************************************************/
	
	/**
	* This function is an implementation of the extendSamples function from the PhD Thesis
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
	extern void simulateSampleVectorUnbUntilDTMC( const sparse * pStateSpace, PTSampleVecUntil pSampleVecUntil,
							const int new_sample_size, const int new_simulation_depth,
							const bitset * pGoodStates, const bitset * pTransientStates);
	
#endif
