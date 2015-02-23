/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: sample_vec.h,v $
*	$Revision: 1.8 $
*	$Date: 2007/10/14 11:14:51 $
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
*	Source description: This is a header file for the sample-vector
*	structure and operations on it. It is used in the simulation engine.
*/

#include "macro.h"

//Is needed for the runtime.h
#include "bitset.h"

#ifndef SAMPLE_VEC_H

#define SAMPLE_VEC_H
	
	/****************************************************************************/
	/**************THE DEFINITION OF THE SAMPLE OF OBSERVATIONS TYPE*************/
	/****************************************************************************/
	
	/**
	* This is a sample-vector structure, this one is used for the unbounded until simulation.
	* initial_state - the initial observation state index
	* conv_confidence_zeta - the convergence-confidence zeta value:
	*				we expect to have "conv_confidence_zeta >=1"
	* curr_sample_size - The current sample size, i.e. the length of the pObservationsVec array.
	* curr_simulation_depth - the current simulation depth needed or the unbounded until.
	* sum_good - the current number of good absorbing states in the sample
	* sum_trans - the current number of transient states in the sample
	* pObservationsVec - the vector of observations, of size curr_sample_size
	* pTransStateInd - the bitset that contains the indexes of the transient states
	*			in the pObservationsVec vector.
	*/
	typedef struct SSampleVecUntil{
		int initial_state;
		double conv_confidence_zeta;
		int curr_sample_size;
		int curr_simulation_depth;
		int sum_good;
		int sum_trans;
		int * pObservationsVec;
		bitset * pTransStateInd;
	} TSampleVecUntil;
	typedef TSampleVecUntil * PTSampleVecUntil;
	
	/****************************************************************************/
	/************THE ALLOCATION AND DEALLOCATION OF THE SAMPLE VECTOR************/
	/****************************************************************************/
	
	/**
	* This method is used to allocate a new sample-vector structure:
	* We set "curr_sample_size" to "sample_size".
	* We set "curr_simulation_depth" to zero.
	* We set "sum_good" to zero.
	* We set "sum_trans" to "sample_size".
	* We allocate "pTransStateInd" and "pObservationsVec" of size "sample_size".
	* We set "pTransStateInd" to all ones.
	* We fill "pObservationsVec" with "initial_state".
	* @param sample_size the initial sample size
	* @param initial_state the initial observation.
	* @param conv_confidence_zeta the convergence-confidence zeta value:
	*				we expect to have "conv_confidence_zeta >=1"
	* @return the newly-created sample vector.
	*/
	extern inline PTSampleVecUntil allocateSampleVectorUntil( const int sample_size, const int initial_state, double conv_confidence_zeta );
	
	/**
	* This method is used to extend a sample-vector structure:
	* We can only increase the sample size, i.e. we expect
	*	"new_sample_size >= pSampleVecUntil->curr_sample_size"
	* @param pSampleVecUntil the sample-vector scheduled for reallocation.
	* @param sample_size the new sample size
	* @return the newly-created sample vector.
	*/
	extern inline void extendSampleVectorUntil( PTSampleVecUntil pSampleVecUntil, const int new_sample_size );
	
	/****************************************************************************/
	/***************FREE THE MEMORY ALLOCATED FOR THE SAMPLE VECTOR**************/
	/****************************************************************************/
	
	/**
	* This method is used to free the sample-vector structure.
	* @param pSampleVecUntil the sample vector that has to be freed.
	*/
	extern inline void freeSampleVectorUntil( PTSampleVecUntil pSampleVecUntil );
	
	/****************************************************************************/
	/***************************PRINT THE SAMPLE VECTOR**************************/
	/****************************************************************************/
	
	/**
	* This method prints the sample-vector structure for unbounded-until simulation
	* @param pSampleVecUntil the sample-vector to be printed
	*/
	extern inline printSampleVectorUntil( PTSampleVecUntil pSampleVecUntil );
	
#endif
