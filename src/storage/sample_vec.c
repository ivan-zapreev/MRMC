/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: sample_vec.c,v $
*	$Revision: 1.11 $
*	$Date: 2007/10/14 11:14:50 $
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
*	Source description: This is a source file for the sample-vector
*	structure and operations on it. It is used in the simulation engine.
*/

#include "sample_vec.h"
#include "sparse.h"

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
inline PTSampleVecUntil allocateSampleVectorUntil( const int sample_size, const int initial_state,
							double conv_confidence_zeta ){
	PTSampleVecUntil pSampleVecUntil = (PTSampleVecUntil) calloc(1, sizeof(TSampleVecUntil));
	int i = 0;
	
	IF_SAFETY( pSampleVecUntil != NULL )
		//Assign the initial state
		pSampleVecUntil->initial_state = initial_state;
		//Assign the convergence-confidence zeta
		pSampleVecUntil->conv_confidence_zeta = conv_confidence_zeta;
		//Assign the sample size right away
		pSampleVecUntil->curr_sample_size = sample_size;
		//It is allways zero in the beginning
		pSampleVecUntil->curr_simulation_depth = 0;
		
		//Asuming the initial state is transient we
		//have that all states in the sample are
		//identical and transient
		pSampleVecUntil->sum_good = 0;
		pSampleVecUntil->sum_trans = sample_size;
		
		pSampleVecUntil->pObservationsVec = (int*) calloc( sample_size, sizeof( int ) );
		//Can't use memset here since "initial_state" is an integer
		for( i = 0; i < sample_size; i++ ){
			pSampleVecUntil->pObservationsVec[i] = initial_state;
		}
		
		pSampleVecUntil->pTransStateInd = get_new_bitset( sample_size );
		//Sets all bits, including unused bits of the last block, to one!
		fill_bitset_one( pSampleVecUntil->pTransStateInd );
	ELSE_SAFETY
		printf("ERROR: We've run out of memory.\n");
		exit(1);
	ENDIF_SAFETY
	
	return pSampleVecUntil;
}

/**
* This method is used to extend a sample-vector structure:
* We can only increase the sample size, i.e. we expect
*	"new_sample_size >= pSampleVecUntil->curr_sample_size"
* @param pSampleVecUntil the sample-vector scheduled for reallocation.
* @param sample_size the new sample size
* @return the newly-created sample vector.
*/
inline void extendSampleVectorUntil( PTSampleVecUntil pSampleVecUntil, const int new_sample_size ){
	int i = 0;
	
	IF_SAFETY( pSampleVecUntil != NULL )
		const int old_sample_size = pSampleVecUntil->curr_sample_size;
		
		IF_SAFETY( old_sample_size <= new_sample_size )
			const int initial_state = pSampleVecUntil->initial_state;
			pSampleVecUntil->curr_sample_size = new_sample_size;
			
			//Add extra observations
			pSampleVecUntil->pObservationsVec = ( int* ) realloc( pSampleVecUntil->pObservationsVec,
										new_sample_size * sizeof( int ) );
			for( i = old_sample_size; i < new_sample_size ; i++){
				pSampleVecUntil->pObservationsVec[i] = initial_state;
			}
			
			//Add the new transient states
			pSampleVecUntil->sum_trans += new_sample_size - old_sample_size;
			//WARNING: The extended "pSampleVecUntil->pTransStateInd"
			//should have unspoiled unused elements of the last block,
			//I.e. the elements after "pSampleVecUntil->pTransStateInd->n-1"
			//are expected to be set one, since they should not have been
			//touched after this bitset was allocated in "allocateSampleVectorUntil"
			//There we used "fill_bitset_one" that sets all, including unused,
			//bits to one!
			extend_bitset( pSampleVecUntil->pTransStateInd, new_sample_size, TRUE );
		ELSE_SAFETY
			printf("ERROR: Trying to decrease the sample size of PTSampleVecUntil.\n");
			exit(1);
		ENDIF_SAFETY
	ELSE_SAFETY
		printf("ERROR: Trying to reallocate a NULL pointer PTSampleVecUntil.\n");
		exit(1);
	ENDIF_SAFETY
}

/****************************************************************************/
/***************FREE THE MEMORY ALLOCATED FOR THE SAMPLE VECTOR**************/
/****************************************************************************/

/**
* This method is used to free the sample-vector structure.
* @param pSampleVecUntil the sample vector that has to be freed.
*/
inline void freeSampleVectorUntil( PTSampleVecUntil pSampleVecUntil ){
	if( pSampleVecUntil != NULL ){
		if( pSampleVecUntil->pObservationsVec != NULL ){
			free( pSampleVecUntil->pObservationsVec );
		}
		if( pSampleVecUntil->pTransStateInd != NULL ){
			free_bitset( pSampleVecUntil->pTransStateInd );
		}
		free( pSampleVecUntil );
	}
}

/****************************************************************************/
/***************************PRINT THE SAMPLE VECTOR**************************/
/****************************************************************************/

/**
* This method prints the sample-vector structure for unbounded-until simulation
* @param pSampleVecUntil the sample-vector to be printed
*/
inline printSampleVectorUntil( PTSampleVecUntil pSampleVecUntil ){
	if( pSampleVecUntil != NULL ){
		printf("------- Printing PTSampleVecUntil -------\n" );
		printf( "\tinitial_state = %d\n", pSampleVecUntil->initial_state + 1 );
		printf( "\tconv_confidence_zeta = %e\n", pSampleVecUntil->conv_confidence_zeta );
		printf( "\tcurr_sample_size = %d\n", pSampleVecUntil->curr_sample_size );
		printf( "\tcurr_simulation_depth = %d\n", pSampleVecUntil->curr_simulation_depth );
		printf( "\tsum_good = %d\n", pSampleVecUntil->sum_good );
		printf( "\tsum_trans = %d\n", pSampleVecUntil->sum_trans );
		printf("\tpObservationsVec :\n\t\t");
		print_vec_int( pSampleVecUntil->curr_sample_size , pSampleVecUntil->pObservationsVec );
		printf( "\tWARNING: For getting an external (a user-level) state index do +1.\n" );
		printf("\tpTransStateInd :\n\t\t");
		print_bitset_states( pSampleVecUntil->pTransStateInd );
		printf("\n");
	} else {
		printf("\tWARNING: Trying to print the NULL pointer of type PTSampleVecUntil.\n");
	}
}
