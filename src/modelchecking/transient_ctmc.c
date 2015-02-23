/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: transient_ctmc.c,v $
*	$Revision: 1.8 $
*	$Date: 2007/08/17 12:03:29 $
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
*	Source description: Perform Transient Analysis for CSL - X, U.
*/

#include "transient_ctmc.h"

/**
* Make certain states (not in n_absorbing) absorbing.
* Creates a new empty sparse matrix, then assighs non absorbing rows via pointers
* and computes uniformization rates for them. This method removes from n_absorbing
* phi states which belong to BSCCs. So the steady state detection in uniformization
* can be used.
* @param: sparse *state_space: the state space
* @param: bitset *n_absorbing: not absorbing states, all of them are phi states, is modified
*	    in this method.
* @param: double *plambda(ref.): return value of uniformizing rate(also q).
* @param: double *abse: return row_sums
* @return: sparse *: new sparse matrix with some states made absorbing.
* NOTE: makes states absorbing (in ab_state_space) by assigning pointers
*	  to rows (in state_space) only for those rows which are not to be
*	  made absorbing.
* NOTE: initially *lambda should be equal to 0!!!!
*/
static sparse * ab_state_space_ssd(sparse * state_space, bitset *n_absorbing, double *plambda, double *abse, int *non_absorbing)
{
	/*Make not phi or psi state absorbing*/
	sparse * abs_npho_psi = ab_state_space(state_space, n_absorbing, plambda, abse, non_absorbing);
	/*Store state space dimensions*/
	const int size = abs_npho_psi->rows;
	/*Create a TBSCCs structure to search for phi states that belong to BSCCs in abs_npho_psi*/
	TBSCCs * pBSCCsHolder = allocateTBSCCs(abs_npho_psi);
	/*Search for BSCCs among n_absorbing states, all of them are phi states*/
	int ** ppNewBSCCs = getNewBSCCs(pBSCCsHolder, n_absorbing);
	
	int * pbscc_mapping = getStatesBSCCsMapping(pBSCCsHolder);
	int i;
	/*Removes phi states which belong to some bscc from n_absorbing*/
	for( i=0; i<size; i++ ){
		/*If a non absorbing phi state is in some bscc*/
		if( get_bit_val( n_absorbing, i ) && pbscc_mapping[i] != 0){
			set_bit_val(n_absorbing, i, BIT_OFF); /*Remove this state as it should become absorbing*/
		}
	}
	/*Now n_absorbing contains those states that are phi and do not belong to any bscc*/
	
	/*Free temporary structures*/
	free_abs(abs_npho_psi);
	freeTBSCC(pBSCCsHolder);
	freeBSCCs(ppNewBSCCs);
	
	/*Set counter of not absorbing states to 0 as soon as we will
	recalculte this in the next call of ab_state_space*/
	*non_absorbing = 0;
	/*It should be 0 in the call of ab_state_space*/
	*plambda = 0.0;
	/*Clean abse as we will have less values, and they will be set
	in the call of ab_state_space*/
	bzero( abse, sizeof(double)*size );
	
	/*Make phi states that belong to BSCCs in abs_npho_psi absorbing*/
	return ab_state_space(state_space, n_absorbing, plambda, abse, non_absorbing);
}

/**
* Solve the unbounded until operator for all states.
* @param	: bitset *phi: SAT(phi).
* @param	: bitset *psi: SAT(psi).
* @return	: double *: result of the unbounded until operator for all states.
* NOTE: adapted from
* 	 1. F. Ciesinski, M. Grober. On Probabilistic Computation Tree Logic.
* 	 LNCS, Vol. 2925, Springer, pp. 147-188, 2004.
*/
double * unbounded_until(bitset *phi, bitset *psi)
{
	sparse *state_space = get_state_space();
	bitset * EU = get_exist_until(state_space, phi, psi);
	bitset * AU = get_always_until(state_space, phi, psi, EU);
	int i, size = state_space->rows;
	double * initial = (double *)calloc(size,sizeof(double));
	double *rhs = (double *)calloc(size,sizeof(double)), *result=NULL;
	sparse *pQ = init_matrix(size,size);
	bitset *dummy=NULL;
	int *pValidStates=NULL;
	
	dummy = not(EU);
	or_result(AU, dummy);
	not_result(dummy);
	pValidStates=count_set(dummy);
	
	//Make ~(Phi /\ ~Psi) states absorbing
	initMatrix(state_space, pQ, pValidStates);
	//Get the embedded DTMC P
	mult_mtx_cer_const_array(pQ, get_row_sums(), pValidStates, 1);
	
	//Initialize for DTMC: initial and rhs vectors
	for(i=0;i<size;i++)
	{
		if( get_bit_val( AU, i ) ){
			rhs[i] = 1.0;
			initial[i] = 1.0;
		}
	}
	
	//Solve (I-P)x = 1_Psi
	result = unbounded_until_universal(pQ, pValidStates, initial, rhs, TRUE);
	
	//Revert matrix P -> Q
	//NOTE: not required for diagonal elements
	mult_mtx_cer_const_array(pQ, get_row_sums(), pValidStates, 0);
	cleanMatrix(pQ, pValidStates);
	
	free_bitset(EU);
	free_bitset(AU);
	free_bitset(dummy);
	free(rhs);
	free_mtx_sparse(pQ);
	free(pValidStates);
	return result;
}

/**
* Solve the unbounded until operator for all states with lumping.
* @param	: bitset *phi: SAT(phi).
* @param	: bitset *psi: SAT(psi).
* @return	: double *: result of the unbounded until operator for all states.
*/
double * unbounded_until_lumping(bitset *phi, bitset *psi)
{
	sparse *state_space = get_state_space();
	bitset * EU = get_exist_until(state_space, phi, psi);
	bitset * AU = get_always_until(state_space, phi, psi, EU);
	double *initial, *rhs, *result, *lumped_result;
	bitset *dummy, *lumped_dummy;
	int *pValidStates;
	block *B;
	
	// create initial partition for unbounded until formula and lump
	partition *P = init_partition_formula(EU, AU, FALSE);
	sparse *Q = lump(P, state_space);
	
	dummy = not(EU);
	or_result(AU, dummy);
	not_result(dummy);
	
	//Lump the valid states bitset
	lumped_dummy = lump_bitset(P, dummy);
	pValidStates = count_set(lumped_dummy);
	
	//Get the embedded DTMC, NOTE: we do not need to
	//make states absorbing, because it seems to
	//be taken care of during lumping
	double *q_row_sum = get_mtx_row_sums(Q);
	mult_mtx_cer_const_array(Q, q_row_sum, pValidStates, 1);
	
	//Initialize for lumped DTMC: initial and rhs vectors
	initial = (double *)calloc(Q->rows,sizeof(double));
	rhs = (double *)calloc(Q->rows,sizeof(double));
	for(B = P->blocks; B != NULL; B = B->next){
		if( get_bit_val( AU, B->states->index ) ){
			rhs[B->row] = 1.0;
			initial[B->row] = 1.0;
		}
	}
	
	//Solve (I-P)x = 1_Psi, NOTE: we do not need inversion, the last
	//parameter is set to FALSE, because the matrix will be discarded
	lumped_result = unbounded_until_universal(Q, pValidStates, initial, rhs, FALSE);
	
	//Unlump the resulting vector
	result = unlump_vector(P, state_space->rows, lumped_result);
	
	free_bitset(EU);
	free_bitset(AU);
	free_bitset(dummy);
	free_bitset(lumped_dummy);
	free(rhs);
	free(pValidStates);
	free(lumped_result);
	free(q_row_sum);
	free_partition(P);
	free_sparse_ncolse(Q);
	
	return result;
}

/**
* Solve the bounded until operator by uniformization, without steady-state detection
* @param	: bitset *n_absorbing: non-absorbing states.
* @param	: bitset *reach: goal states for instance SAT(psi), i.e the i_\psi vector.
* @param	: double supi: sup I
* @return	: double *: result of the unbounded until operator for all states.
* NOTE: 1. J.-P. Katoen, M.Z. Kwiatowska, G. Norman, D.A. Parker.
* 	  Faster and symbolic CTMC model checking. LNCS Vol. 2165, Springer,
* 	  Aachen, Germany, pp. 23-38, 2001.
*/
static double * uniformization_plain(bitset *n_absorbing, double *reach, double supi)
{
	sparse *state_space = get_state_space();
	const double eps=get_error_bound();
	const double u=get_underflow();
	const double o=get_overflow();
	const int size = state_space->rows;
	/*Store the initial pointer to the reach as it will be freed somewhere else*/
	const double * golden_reache = reach;
	
	double lambda=0, current_fg=0.0;
	int i, j, k, non_absorbing=0;
	
	/*diag is used to store diagonal values from the abs matrix*/
	double * diag = (double *)calloc(size, sizeof(double));
	/*res is used to store iterates*/
	double * res = (double *)calloc(size, sizeof(double));
	/*result is used to store partial sum*/
	double * result = (double *)calloc(size, sizeof(double));
	
	/*Make states absorbing: not phi || psi*/
	sparse *abs = ab_state_space(state_space, n_absorbing, &lambda, diag, &non_absorbing);
	/*allocate the array which will hold ids of valid rows from abs matrix*/
	int * valid_rows = (int *)calloc(non_absorbing, sizeof(int));
	
	//print_mtx_sparse(state_space);
	//print_mtx_sparse(abs);
	
	/*Find valid rows*/
	for(i=0, j=0; i<size; i++) {
		if( get_bit_val( n_absorbing, i ) ) {
			valid_rows[j++]=i;
		}
	}
	/*This copying is vital because we need to preserve 1.0 values of absorbing states*/
	memcpy( res, reach, sizeof(double)*size );
	memcpy( result, reach, sizeof(double)*size );
	
	/*Initially, vectors res = reach!*/
	FoxGlynn * pFG = NULL;
	//printf("lambda = %1.15le, supi = %1.15le, u = %1.15le, o = %1.15le, eps = %1.15le\n",lambda, supi, u, o, eps);
	if( fox_glynn(lambda*supi, u, o, eps, &pFG) ) {
		printf("Fox-Glynn: ltp = %d, rtp = %d, w = %1.15le\n",pFG->left, pFG->right, pFG->total_weight);
		
		/*R-E(s)*/
		sub_mtx_diagonal(abs, diag);
		/*Uniformize : lambda>0*/
		mult_mtx_const(abs, 1/lambda);
		add_mtx_cons_diagonal(abs, 1.0);
		
		//print_mtx_sparse(abs);
		
		/*This variable will be used to store a tmp pointer to an array*/
		double * tmp_arr = NULL;
		
		/*Skip upto left*/
		for( i=1; i < pFG->left; i++ ) {
			multiply_mtx_cer_MV(abs, reach, res, non_absorbing, valid_rows);
			/*Flip pointers*/
			tmp_arr = reach; reach = res; res = tmp_arr;
		}
		
		/*Compute upto right*/
		int * iterator;
		for( ; i <= pFG->right; i++ ) {
			current_fg = pFG->weights[i - pFG->left];
			multiply_mtx_cer_MV(abs, reach, res, non_absorbing, valid_rows);
			iterator = valid_rows;
			for( j=0; j < non_absorbing; j++, iterator++ ) {
				result[*iterator] += current_fg * res[*iterator];
			}
			/*Flip pointers*/
			tmp_arr = reach; reach = res; res = tmp_arr;
		}
		/*Divide with total weight*/
		iterator = valid_rows;
		for( j=0; j < non_absorbing; j++, iterator++ ) {
			result[*iterator] /= pFG->total_weight;
		}
		
		/*Reset the matrix to its original state
		NOTE: operations on diagonals are not required */
		mult_mtx_const(abs, lambda);
	}
	
	//Free the FogGlynn structure
	freeFG(pFG); pFG = NULL;
	
	/*Free dummy vectors*/
	free( valid_rows );
	free( diag );
	/*Free an array allocated here but not in the external function*/
	if( golden_reache == res )
		free( reach );
	else
		free( res );
	free_abs( abs );
	
	return result;
}

/**
* Detects steady-state while uniformization for CSL logic.
* @param: double * reach_psi an array of probabilities to reach psi states
* 	    for some iteration
* @param: double * reach_bad an array of probabilities to reach
* 	   (not phi and not psi) U (phi state in some bscc) states
* 	   for some iteration
* @param: const double delta the error threshold
* @param: const int valid_size the length valid_states array
* @param: int* valid_states array with ids of transient states
* @return: TRUE if steady state is reached, FALSE otherwise
* NOTE: We use the property that the probabilityh to be in transient state
* 		  becomes 0 in the long run
*/
static BOOL isSteadyState(double * reach_psi, double * reach_bad, const double delta,
                          const int valid_size, int* valid_states)
{
	int i = 0;
	
	for( i=0; i < valid_size; i++)
	{
		//Test the probability to remane in transient
		//states when starting in state i
		if( ( 1 - reach_psi[valid_states[i]] - reach_bad[valid_states[i]] ) > delta)
		return FALSE;
	}
	
	return TRUE;
}

/**
* Solve the bounded until operator by uniformization with steady-state detection
* @param: bitset *n_absorbing: non-absorbing states.
* @param: bitset *psi : psi states for steady-state detection
* @param: bitset *reach_psi: goal states for instance SAT(psi), i.e the i_\psi vector.
* @param: double supi: sup I
* @return: double *: result of the unbounded until operator for all states.
* NOTE: 1. J.-P. Katoen, M.Z. Kwiatowska, G. Norman, D.A. Parker.
* 	  Faster and symbolic CTMC model checking. LNCS Vol. 2165, Springer,
* 	  Aachen, Germany, pp. 23-38, 2001.
*/
static double * uniformization_ssd(bitset *n_absorbing, bitset *psi, double *reach_psi, double supi)
{
	sparse *state_space = get_state_space();
	const double eps=get_error_bound();
	const double u=get_underflow();
	const double o=get_overflow();
	const int size = state_space->rows;
	/*Store the initial pointer to the reach_psi as it will be freed somewhere else*/
	const double * golden_reache_psi = reach_psi;
	
	double lambda=0, current_fg=0.0;
	int i, j, k, non_absorbing=0;
	
	/*diag is used to store diagonal values from the abs matrix*/
	double * diag = (double *)calloc(size, sizeof(double));
	/*res_psi is used to store iterates res_psi = P*reach_psi*/
	double * res_psi = (double *)calloc(size, sizeof(double));
	/*res_bad is used to store iterates, res_bad = P*reach_bad*/
	double * res_bad = (double *)calloc(size, sizeof(double));
	/*reach_bad is used to store iterates*/
	double * reach_bad = (double *)calloc(size, sizeof(double));
	/*result is used to store partial sum*/
	double * result = (double *)calloc(size, sizeof(double));
	
	/*Make states absorbing: not phi || psi, and then those phi that will be remaining in some bscss*/
	sparse *abs = ab_state_space_ssd(state_space, n_absorbing, &lambda, diag, &non_absorbing);
	/*allocate the array which will hold ids of valid rows from abs matrix*/
	int * valid_rows = (int *)calloc(non_absorbing, sizeof(int));
	
	/*Construct the initial vector for the absorbing states which are not Psi states, i.e. bad_states*/
	bitset * bad_states = or(n_absorbing,psi);
	not_result(bad_states);
	
	//printf("bad_states : ");
	//print_bitset_states(bad_states);
	
	/*Find valid rows and also create an initial vector for bad states*/
	for( i=0, j=0; i<size; i++ ) {
		/*Find valid rows*/
		
		if( get_bit_val( n_absorbing, i ) ) {
			valid_rows[j++]=i;
		}
		/*Create an initial vector for bad states*/
		if( get_bit_val( bad_states, i ) ) {
			reach_bad[i] = 1.0;
		}
	}
	/*This copying is vital because we need to preserve 1.0 values of absorbing states*/
	memcpy( res_bad, reach_bad, sizeof(double)*size );
	memcpy( res_psi, reach_psi, sizeof(double)*size );
	memcpy( result, reach_psi, sizeof(double)*size );
	
	/*Initially, vectors res = reach!*/
	FoxGlynn * pFG = NULL;
	/*Note that for steady-state detection we need to take error bound
	eps/2 for the Fox-Glynn algorithm*/
	if( fox_glynn(lambda*supi, u, o, eps / 2.0, &pFG) ) {
		printf("Fox-Glynn: ltp = %d, rtp = %d, w = %1.15le\n", pFG->left, pFG->right, pFG->total_weight);
		
		/* R-E(s) */
		sub_mtx_diagonal(abs,diag);
		/* uniformize : lambda>0 */
		mult_mtx_const(abs, 1/lambda);
		add_mtx_cons_diagonal(abs, 1.0);
		
		//print_mtx_sparse(abs);
		
		/*becomes TRUE when the steady-state is reached*/
		BOOL isSS = FALSE;
		const double delta = eps / 4.0;
		/*This variable will be used to store a tmp pointer to an array*/
		double * tmp_arr = NULL;
		/*This is a magic M constant, we check for ssd every M iterations*/
		const int M = 10;
		
		/*Skip upto left trancation point*/
		for( i=1; i < pFG->left; i++ ) {
			multiply_mtx_cer_MV(abs, reach_psi, res_psi, non_absorbing, valid_rows);
			multiply_mtx_cer_MV(abs, reach_bad, res_bad, non_absorbing, valid_rows);
			if( ( i % M == 0 ) && ( isSS = isSteadyState(res_psi, res_bad, delta, non_absorbing, valid_rows) ) ) {
				/*The steady-state has been reached before the left truncation point*/
				memcpy( result, res_psi, sizeof(double)*size );
				printf("SSD: i = %d < fg.left\n", i);
				break;
			}
			/*Flip pointers*/
			tmp_arr = reach_psi; reach_psi = res_psi; res_psi = tmp_arr;
			tmp_arr = reach_bad; reach_bad = res_bad; res_bad = tmp_arr;
		}
		
		/*If the steady state has not been reached, then compute upto right truncation point*/
		if( !isSS ) {
			double part_sum = 0; /*will hold sum of poisson probs from left truncation point to steady-state point*/
			int *iterator;
			for( ; i <= pFG->right; i++ ) {
				current_fg = pFG->weights[i - pFG->left];
				multiply_mtx_cer_MV(abs, reach_psi, res_psi, non_absorbing, valid_rows);
				multiply_mtx_cer_MV(abs, reach_bad, res_bad, non_absorbing, valid_rows);
				iterator = valid_rows;
				for( j=0; j < non_absorbing; j++, iterator++ ) {
					result[*iterator] += current_fg * res_psi[*iterator];
				}
				part_sum += current_fg; /*Accumulate weights, because the SSD may be reached here, then we will need them*/
				if( ( i % M == 0 ) && ( isSS = isSteadyState(res_psi, res_bad, delta, non_absorbing, valid_rows) ) ) {
					printf("SSD: i = %d, part_sum = %1.15le\n", i, part_sum);
					break;
				}
				/*Flip pointers*/
				tmp_arr = reach_psi; reach_psi = res_psi; res_psi = tmp_arr;
				tmp_arr = reach_bad; reach_bad = res_bad; res_bad = tmp_arr;
			}
			
			/*The part after the steady-state point*/
			if( isSS ) {
				double mult = pFG->total_weight - part_sum;
				iterator = valid_rows;
				for( j=0; j < non_absorbing; j++, iterator++ ) {
					result[*iterator] += mult * res_psi[*iterator];
				}
			}
			/*Divide with total weight */
			iterator = valid_rows;
			for( j=0; j < non_absorbing; j++, iterator++ ) {
				result[*iterator] /= pFG->total_weight;
			}
		}
		/*Reset the matrix to its original state
		NOTE: operations on diagonals are not required */
		mult_mtx_const(abs,lambda);
	}
	
	//Free the FogGlynn structure
	freeFG(pFG); pFG = NULL;
	
	/*Free dummy vectors*/
	free( valid_rows );
	free( diag );
	/*Free an array allocated here but not in the external function*/
	if( golden_reache_psi == res_psi ) {
		free( reach_psi );
	} else {
		free( res_psi );
	}
	free( res_bad );
	free( reach_bad );
	free_bitset( bad_states );
	free_abs( abs );
	
	return result;
}

/**
* Solve the bounded until operator by uniformization, this method is a wrapper
* depending on setting it either calls a plain uniformization or uses uniformization
* with a steady-state detection
* @param: bitset *n_absorbing: non-absorbing states.
* @param: bitset * psi: psi states for steady-state detection, may be NULL if (ssd_allowed == * FALSE)
* @param: bitset *reach: goal states for instance SAT(psi), i.e the i_\psi vector.
* @param: double supi: sup I
* @param: BOOL ssd_allowed: true if steady state detection is allowed to be used, it does not
* 	   mean that it will be used though, the later depends on runtime settings, by default
* 	   ssd it is off.
* @return		: double *: result of the unbounded until operator for all states.
* NOTE: 1. J.-P. Katoen, M.Z. Kwiatowska, G. Norman, D.A. Parker.
* 	  Faster and symbolic CTMC model checking. LNCS Vol. 2165, Springer,
* 	  Aachen, Germany, pp. 23-38, 2001.
*/
static double * uniformization(bitset *n_absorbing, bitset *psi, double *reach, double supi, BOOL ssd_allowed)
{
  if( ssd_allowed && is_ssd_on() ) {
	if(psi == NULL) {
		printf("ERROR: The steady-state detection is on, but psi states (for phi U[0,t] psi) are not set.\n");
		exit(1);
	}
	return uniformization_ssd(n_absorbing, psi, reach, supi);
  }else{
	return uniformization_plain(n_absorbing, reach, supi);
  }
}

/**
* Universal part of the bounded until operator.
* @param: bitset *good_phi_phi: SAT(phi_and_not_psi) without
* 	    phi states from which you never reach psi states.
* @param: bitset *psi: SAT(psi).
* @param: double supi: sup I
* @return: double *: result of the unbounded until operator for all states.
* NOTE: 1. J.-P. Katoen, M.Z. Kwiatowska, G. Norman, D.A. Parker.
* 	  Faster and symbolic CTMC model checking. LNCS Vol. 2165, Springer,
* 	  Aachen, Germany, pp. 23-38, 2001.
*/
static double * bounded_until_universal(bitset *good_phi_states, bitset *psi, double supi)
{
	sparse *state_space = get_state_space();
	int i, size = state_space->rows;
	double *reach = (double *)calloc(size, sizeof (double)), *result;
	
	//Create the initial vector for backward computations
	for(i=0;i<size;i++) {
		if( get_bit_val( psi, i ) ) {
			reach[i] = 1.0;
		}
	}
	//Do uniformization, compute probabilities, normally, without the
	//optimization we would just use phi_and_not_psi instead of good_phi_states.
	result=uniformization(good_phi_states, psi, reach, supi, TRUE);
	
	free(reach);
	
	return result;
}

/**
* Solve the bounded until operator.
* @param	: bitset *phi: SAT(phi).
* @param	: bitset *psi: SAT(psi).
* @param	: double supi: sup I
* @return	: double *: result of the unbounded until operator for all states.
* NOTE: 1. J.-P. Katoen, M.Z. Kwiatowska, G. Norman, D.A. Parker.
* 	Faster and symbolic CTMC model checking. LNCS Vol. 2165, Springer,
* 	Aachen, Germany, pp. 23-38, 2001.
*/
double * bounded_until(bitset *phi, bitset *psi, double supi)
{
	sparse *state_space = get_state_space();
	double *result;
	
	//Do optimization, we exclude states from Phi which are Psi states and
	//also states from which you allways go to bad i.e. not Phi ^ not Psi states
	//This is only possible if until has lower time bound 'subi'
	//(and reward bound 'subj' if any) equal to 0.
	bitset *good_phi_states = get_good_phi_states( phi, psi, state_space);

	result = bounded_until_universal(good_phi_states, psi, supi);
	
	//Free allocated memory
	free_bitset(good_phi_states);
	
	return result;
}

/**
* Solve the bounded until operator with lumping.
* @param: bitset *phi: SAT(phi).
* @param: bitset *psi: SAT(psi).
* @param: double supi: sup I
* @return: double *: result of the unbounded until operator for all states.
*/
double * bounded_until_lumping(bitset *phi, bitset *psi, double supi)
{
	sparse *original_state_space = get_state_space();
	double *result, *lumped_result;
	
	//Do optimization, we exclude states from Phi which are Psi states and
	//also states from which you allways go to bad i.e. not Phi ^ not Psi states
	//This is only possible if until has lower time bound 'subi'
	//(and reward bound 'subj' if any) equal to 0.
	bitset *good_phi_states = get_good_phi_states( phi, psi, original_state_space);
	
	//Create initial partition for bounded until formula and lump
	partition *P = init_partition_formula(good_phi_states, psi, FALSE);
	sparse *Q = lump(P, original_state_space);
	
	//Free the row sums vector because otherwise it will be lost
	free_row_sums();
	//Set lumped CTMC to be the state space, NOTE: resets the row_sums!
	set_state_space(Q);
	
	//Lump the bitsets
	bitset *lumped_good_phi_states = lump_bitset(P, good_phi_states);
	bitset *lumped_psi = lump_bitset(P, psi);
	
	lumped_result = bounded_until_universal(lumped_good_phi_states, lumped_psi, supi);
	
	//Unlump the resulting vector
	result = unlump_vector(P, original_state_space->rows, lumped_result);
	
	//Free allocated memory
	free_bitset(good_phi_states);
	free_bitset(lumped_good_phi_states);
	free_bitset(lumped_psi);
	free(lumped_result);
	free_partition(P);
	//Free the lumped state space.
	free_sparse_ncolse(Q);
	//You have to do that not to waste the allocated memory
	free_row_sums();
	
	//Restore the original state space, NOTE: resets the row_sums!
	set_state_space(original_state_space);
	
	return result;
}

/**
* @param: bitset *phi: SAT(phi).
* @param: bitset *psi: SAT(psi).
* @param: double subi: sub I
* @param: double supi: sup I
* @return: double *: result of the interval until operator for all states.
* NOTE: 1. C.Baier, B.R. Haverkort, H. Hermanns and J.-P. Katoen.
* 	Model Checking Algorithms for Continuous-Time Markov Chains.
* 	IEEE Transactions on Software Engineering, Vol. 29, No. 6,
* 	pp. 299-316, 2000.
*/
double * interval_until(bitset *phi, bitset *psi, double subi, double supi)
{
	const int size = phi->n;
	int i;
	sparse * state_space = get_state_space();
	double * reach = (double *)calloc(size, sizeof(double));
	double * result1 = NULL, * result2 = NULL;

	//Do optimization, we exclude states from Phi which are Psi states and
	//also states from which you allways go to bad i.e. not Phi ^ not Psi states
	//This is only possible if until has lower time bound 'subi'
	//(and reward bound 'subj' if any) equal to 0.
	//NOTE: In this particular case it is smartly used, so it is safe here!
	bitset *good_phi_states = get_good_phi_states( phi, psi, state_space);
	
	//Create the initial vector for backward computations
	for(i=0;i<size;i++) {
		if( get_bit_val( psi, i ) ) {
			reach[i] = 1.0;
		}
	}
	//Here phi_and_not_psi is a set of non absorbing states
	//printf("supi = %1.15le, subi = %1.15le, supi-subi = %1.15le\n",supi, subi, supi-subi);
	result2 = uniformization(good_phi_states, NULL, reach, (supi-subi), FALSE);

	//It seems like we can not just do another uniformization
	//in a backward manner with this vector result2
	//We should first make clear that we are interested in
	//only PHI states to be acconded, this is done by the next cycle.
	for(i=0;i<size;i++) {
		if( ! get_bit_val( phi, i ) ) {
			result2[i] = 0.0;
		}
	}

	//Here good_phi_states is a set of non absorbing states
	//This bitset is extended with phi && psi states, because
	//were interested in ALL phi states which can reach a psi state
	bitset *phi_and_psi = and(phi,psi);
	or_result(phi_and_psi, good_phi_states);
	result1 = uniformization(good_phi_states, NULL, result2, subi, FALSE);
	
	//Free allocated memory
	free_bitset(good_phi_states);
	free_bitset(phi_and_psi);
	free(result2);
	free(reach);

	return result1;
}

/**
* Solve the interval until operator with lumping.
* @param: bitset *phi: SAT(phi).
* @param: bitset *psi: SAT(psi).
* @param: double subi: sub I
* @param: double supi: sup I
* @return: double *: result of the interval until operator for all states.
*/
double * interval_until_lumping(bitset *phi, bitset *psi, double subi, double supi)
{
	sparse *original_state_space = get_state_space();
	
	//Create initial partition for interval until formula and lump
	partition *P = init_partition_formula(phi, psi, TRUE);
	sparse *Q = lump(P, original_state_space);
	
	//Free the row sums vector because otherwise it will be lost
	free_row_sums();
	//Set lumped CTMC to be the state space, NOTE: resets the row_sums!
	set_state_space(Q);
	
	//Create lumped_phi and lumped_psi bitsets
	bitset *lumped_psi = lump_bitset(P, psi);
	bitset *lumped_phi = lump_bitset(P, phi);
	
	double *lumped_result = interval_until(lumped_phi, lumped_psi, subi, supi);
	
	//Unlump the resulting vector
	double *result = unlump_vector(P, original_state_space->rows, lumped_result);
	
	free_bitset(lumped_phi);
	free_bitset(lumped_psi);
	free(lumped_result);
	free_partition(P);
	//Free the lumped state space.
	free_sparse_ncolse(Q);
	//You have to do that not to waste the allocated memory
	free_row_sums();

	//Restore the original state space, NOTE: resets the row_sums!
	set_state_space(original_state_space);
	
	return result;
}

/**
* Solve the unbounded next operator.
* @param: bitset *phi: SAT(phi).
* @return: double *: result of the unbounded next operator for states.
*/
double * unbounded_next(bitset *phi)
{
	sparse *state_space = get_state_space();
	int size = state_space->rows, i;
	double * vec = (double *)calloc(size, sizeof(double));
	double * res = (double *)calloc(size, sizeof(double));
	
	for(i=0;i<size;i++) {
		if( get_bit_val( phi, i ) ) {
			vec[i] = 1.0;
		}
	}
	set_sparse(state_space);
	multiplyMV(vec, res);	
	free(vec);
	
	const double *e = get_row_sums();	//Note: DO NOT FREE the array you have aquired!
						//It is globally stored in runtime.c
	for(i=0;i<size;i++){
		if( e[i] ) {
			res[i]/=e[i];
		}
	}
	
	return res;
}

/**
* Solve the bounded next operator.
* @param: bitset *phi: SAT(phi).
* @param: double supi: sup I
* @return: double *: result of the bounded next operator for all states.
*/
double * bounded_next(bitset *phi, double supi)
{
	sparse *state_space = get_state_space();
	int size = state_space->rows, i;
	double * vec = (double *)calloc(size, sizeof(double)), vec_t;
	double * res = (double *)calloc(size, sizeof(double));
	
	for(i=0;i<size;i++) {
		if( get_bit_val( phi, i ) ) {
			vec[i] = 1.0;
		}
	}
	set_sparse(state_space);
	multiplyMV(vec, res);	
	free(vec);
	
	const double *e = get_row_sums();	//Note: DO NOT FREE the array you have aquired!
						//It is globally stored in runtime.c
	for(i=0;i<size;i++) {
		if(vec_t = e[i]) { //This is done on purpose, we assign e[i] to vec_t
			res[i]=(res[i]*(1-exp(-vec_t*supi)))/vec_t;
		}
	}
	
	return res;
}

/**
* Solve the interval next operator.
* @param: bitset *phi: SAT(phi).
* @param: double subi: sub I
* @param: double supi: sup I
* @return: double *: result of the interval next operator for all states.
*/
double * interval_next(bitset *phi, double subi, double supi)
{
	sparse *state_space = get_state_space();
	int size = state_space->rows, i;
	double * vec = (double *)calloc(size, sizeof(double)), vec_t;
	double * res = (double *)calloc(size, sizeof(double));
	
	for(i=0;i<size;i++) {
		if( get_bit_val( phi, i )) {
			vec[i] = 1.0;
		}
	}
	set_sparse(state_space);
	multiplyMV(vec, res);
	free(vec);
	
	const double *e = get_row_sums();	//Note: DO NOT FREE the array you have aquired!
						//It is globally stored in runtime.c
	for(i=0;i<size;i++) {
		if(vec_t = e[i]) {//This is done on purpose, we assign e[i] to vec_t
			res[i]=(res[i]*(exp(-vec_t*subi)-exp(-vec_t*supi)))/vec_t;
		}
	}
	
	return res;
}
