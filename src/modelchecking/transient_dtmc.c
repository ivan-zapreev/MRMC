/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: transient_dtmc.c,v $
*	$Revision: 1.6 $
*	$Date: 2007/08/17 12:03:30 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Maneesh Khattri, Ivan Zapreev, Tim Kemna
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
*	Source description: Perform Transient Analysis for PCTL - X, U.
*/

#include "transient_dtmc.h"

/**
* Creates new R matrix and copies all the rows that belong to the pValidStates into it
* @param pStateSpace the initial matrix
* @param pP the new matrix to where to copy rows
* @param pValidStates the valid states i.e. ids of the rows to be copied
*                     this array contains the number of nodes as the first element
*/
static void makeAbsorbingDTMC(const sparse * pStateSpace, sparse * pP, int * pValidStates)
{
	int i;
	int * pNCols = pStateSpace->ncols;
	values * pRows = pStateSpace->val;
	int valid_state;
	
	/*Set diagoral velues to 1 for all*/
	const int size = pStateSpace->rows;
	for( i = 0; i < size  ; i++ )
	{
		pP->val[i].diag = 1;
	}
	
	/*Form the pP matrix, here the 1 values of the pValidStates get correct values*/
	for( i = 1; i <= pValidStates[0] ; i++ )
	{
		valid_state = pValidStates[i];
		set_mtx_row( pP, valid_state, pNCols[valid_state], pRows[valid_state] );
	}
}

/**
* Solve the unbounded until operator for all states for DTMC.
* @param		: bitset *phi: SAT(phi).
* @param		: bitset *psi: SAT(psi).
* @return	: double *: result of unbounded until operator for all states.
* NOTE: 1. F. Ciesinski, M. Grober. On Probabilistic Computation Tree Logic.
* 	  LNCS, Vol. 2925, Springer, pp. 147-188, 2004.
*/
double * dtmc_unbounded_until(bitset *phi, bitset *psi)
{
	sparse *state_space = get_state_space();
	bitset * EU = get_exist_until(state_space, phi, psi);
	bitset * AU = get_always_until(state_space, phi, psi, EU);
	int i, size = state_space->rows;
	double * initial = (double *)calloc(size,sizeof(double));
	double *rhs = (double *)calloc(size,sizeof(double)), *result=NULL;
	sparse *pP = init_matrix(size,size);
	bitset *dummy=NULL;
	int *pValidStates=NULL;
	
	dummy = not(EU);
	or_result(AU, dummy);
	not_result(dummy);
	pValidStates=count_set(dummy);
	/* make ~(Phi /\ ~Psi) states absorbing */
	makeAbsorbingDTMC(state_space, pP, pValidStates);
	
	for(i=0;i<size;i++) {
		if( get_bit_val( AU, i ) ){
			rhs[i] = 1.0;
			initial[i] = 1.0;
		}
	}
	/* solve (I-P)x = i_Psi */
	result = unbounded_until_universal(pP, pValidStates, initial, rhs, TRUE);
	
	free(rhs);
	free_bitset(EU);
	free_bitset(AU);
	free_bitset(dummy);
	free_mtx_wrap(pP);
	free(pValidStates);
	
	return result;
}

/**
* Solve the unbounded until operator for all states for DTMC with lumping.
* @param		: bitset *phi: SAT(phi).
* @param		: bitset *psi: SAT(psi).
* @return		: double *: result of unbounded until operator for all states.
*/
double * dtmc_unbounded_until_lumping(bitset *phi, bitset *psi)
{
	sparse *state_space = get_state_space();
	int size = state_space->rows;
	bitset * EU = get_exist_until(state_space, phi, psi);
	bitset * AU = get_always_until(state_space, phi, psi, EU);
	double *initial, *rhs, *result, *lumped_result=NULL;
	bitset *dummy, *lumped_dummy;
	int *pValidStates=NULL;
	block *B;
	
	// create initial partition for unbounded until formula and lump
	partition *P = init_partition_formula(EU, AU, FALSE);
	sparse *pP = lump(P, state_space);
	
	dummy = not(EU);
	or_result(AU, dummy);
	not_result(dummy);
	lumped_dummy = lump_bitset(P, dummy);
	pValidStates=count_set(lumped_dummy);
	
	/* initialize for lumped DTMC: initial and rhs vectors */
	initial = (double *)calloc(pP->rows,sizeof(double));
	rhs = (double *)calloc(pP->rows,sizeof(double));
	for(B = P->blocks; B != NULL; B = B->next){
		if( get_bit_val( AU, B->states->index ) ){
			rhs[B->row] = 1.0;
			initial[B->row] = 1.0;
		}
	}
	/* solve (I-P)x = i_Psi */
	lumped_result = unbounded_until_universal(pP, pValidStates, initial, rhs, FALSE);
	
	// transform result vector
	result = unlump_vector(P, size, lumped_result);
	
	free(rhs);
	free(lumped_result);
	free_bitset(EU);
	free_bitset(AU);
	free_bitset(dummy);
	free_bitset(lumped_dummy);
	free_sparse_ncolse(pP);
	free_partition(P);
	free(pValidStates);
	
	return result;
}

/**
* The common part of the dtmc_bounded_until method for
* the case with and without lumping.
* @param pP the transition matrix
* @param ppInOutData the pointer to the pointer of the array of values.
*			  We have to do it this way since then the *ppInOutData
*			  array is used later and we can free it as result_2 here.
*			  So the remaining result_1 array is assigned to *ppInOutData.
* @param supi the time bound - number of iterations.
*/
static void dtmc_bounded_until_universal(sparse *pP, double **ppInOutData, double supi)
{
	int i;
	double *result_2, *pTmp;
	double * result_1 = *ppInOutData; //Access the data-array pointer value
	
	/*Compute Q^supi*i_psi*/
	result_2 = (double *)calloc(pP->rows, sizeof (double)), pTmp = NULL;
	for(i = 1; i <= supi ; i++) {
		multiply_mtx_MV( pP, result_1, result_2);
		pTmp = result_1;
		result_1 = result_2;
		result_2 = pTmp;
	}
	
	//Free allocated memory
	free(result_2);
	//IMPRTANT! WENEED THE RIGHT POINTER BACK!
	*ppInOutData = result_1; //Store the remaining pointer
}

/**
* Solve the bounded until operator for DTMC.
* @param		: bitset *phi: SAT(phi).
* @param		: bitset *psi: SAT(psi).
* @param		: double supi: sup I should contain the Natural number
* @return	: double *: result of the unbounded until operator for all states.
* NOTE: 1. F. Ciesinski, M. Grober. On Probabilistic Computation Tree Logic.
* 	  LNCS, Vol. 2925, Springer, pp. 147-188, 2004.
*/
double * dtmc_bounded_until(bitset *phi, bitset *psi, double supi)
{
	sparse * state_space = get_state_space();
	sparse *pP;
	int i;
	const int size = state_space->rows;
	double *result_1 = (double *)calloc(state_space->rows, sizeof (double));
	int * pValidStates;
	
	//Do optimization, we exclude states from Phi which are Psi states and
	//also states from which you allways go to bad i.e. not Phi ^ not Psi states
	//This is only possible if until has lower time bound 'subi'
	//(and reward bound 'subj' if any) equal to 0.
	bitset *good_phi_states = get_good_phi_states( phi, psi, state_space);

	pValidStates = count_set(good_phi_states);
	
	pP = init_matrix(size,size);
	makeAbsorbingDTMC(state_space, pP, pValidStates);
	
	/*init result_1 with i_psi*/
	for(i=0;i<size;i++) {
		if( get_bit_val( psi, i ) ) {
			result_1[i] = 1.0;
		}
	}
	
	/*Compute Q^supi*i_psi*/	
	dtmc_bounded_until_universal(pP, &result_1, supi);
	
	//Free allocated memory
	free_bitset(good_phi_states);
	free(pValidStates);
	free_mtx_wrap(pP);
	
	return result_1;
}

/**
* Solve the bounded until operator for DTMC with lumping.
* @param		: bitset *phi: SAT(phi).
* @param		: bitset *psi: SAT(psi).
* @param		: double supi: sup I should contain the Natural number
* @return		: double *: result of the unbounded until operator for all states.
*/
double * dtmc_bounded_until_lumping(bitset *phi, bitset *psi, double supi)
{
	sparse * state_space = get_state_space();
	block *B;
	
	//Do optimization, we exclude states from Phi which are Psi states and
	//also states from which you allways go to bad i.e. not Phi ^ not Psi states
	//This is only possible if until has lower time bound 'subi'
	//(and reward bound 'subj' if any) equal to 0.
	bitset *good_phi_states = get_good_phi_states( phi, psi, state_space);
	
	// create initial partition for bounded until formula and lump
	partition *P = init_partition_formula(good_phi_states, psi, FALSE);
	sparse *Q = lump(P, state_space);
	
	/*init lumped_result with i_psi*/
	double *lumped_result = (double *)calloc(Q->rows, sizeof (double));
	
	for(B = P->blocks;B!=NULL;B=B->next) {
		if( get_bit_val(psi,B->states->index ) ) {
			lumped_result[B->row] = 1.0;	// if B is labeled with psi
		}
	}
	
	/*Compute Q^supi*i_psi*/
	dtmc_bounded_until_universal(Q, &lumped_result, supi);
	
	// transform result vector
	double *result = unlump_vector(P, state_space->rows, lumped_result);
	
	//Free allocated memory
	free(lumped_result);
	free_sparse_ncolse(Q);
	free_partition(P);
	free_bitset(good_phi_states);
	
	return result;
}

/**
* This method modelchecks the Xphi formula for the DTMC
* @param the phi states
* @return the probabilities
*/
double * dtmc_unbounded_next(bitset *phi)
{
	sparse *state_space = get_state_space();
	const int size = state_space->rows;
	int i;
	double * result = (double *)calloc(size, sizeof(double));
	double * i_phi = (double *)calloc(size, sizeof(double));
	
	for(i=0;i<size;i++) {
		if( get_bit_val( phi, i ) ) {
			i_phi[i] = 1.0;
		}
	}
	
	multiply_mtx_MV(state_space,i_phi, result);
	
	free(i_phi);
	
	return result;
}
