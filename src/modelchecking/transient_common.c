/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: transient_common.c,v $
*	$Revision: 1.7 $
*	$Date: 2007/10/11 16:15:28 $
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
*	Source description: Contains some common methods for transient
*				 analysis of PCTL/CSL/PRCTL/CSRL - X, U.
*/

#include "transient_common.h"

/**
* Solve E(phi U psi) until formula.
* @param: sparse *state_space: the state space
* @param: bitset *phi: satisfaction relation for phi formula.
* @param: bitset *psi: satisfaction relation for psi formula.
* @return: bitset *: result of E(SAT(phi) U SAT(psi)) for all states.
* NOTE: adapted for t-bounded-until from
*	  1. F. Ciesinski, M. Grober. On Probabilistic Computation Tree Logic.
*	  LNCS, Vol. 2925, Springer, pp. 147-188, 2004.
*/
bitset * get_exist_until(const sparse *state_space, const bitset *phi, const bitset *psi)
{
	int i, j, size = phi->n, reach=0, pcols;
	double val;
	int *back_set;
	int * states = NULL;
	bitset * dummy = get_new_bitset(size);
	bitset *EU = or(dummy, psi);
	free_bitset(dummy);
	set_sparse(state_space);
	//Find out and store all psi states first in the states structure
	//The reach variable will contain the number of psi states
	for(i=0;i<size;i++) {
		if( get_bit_val( psi, i ) ) {
			states = (int *) realloc(states, (reach+1)*sizeof(int));
			states[reach++] = i;
		}
	}
	//
	for(i=0;i<reach;i++)
	{
		//Get the number of states from which we can reach
		//the given psi state
		pcols = state_space->pcols[states[i]];
		//Get the set of states from which we can reach
		//the given psi state
		back_set = state_space->val[states[i]].back_set;
		for(j=0;j<pcols;j++) {
			//If we can reach psi state from the phi state
			//and it's not yet been found then
			if( get_bit_val( phi, back_set[j] ) && ( ! get_bit_val( EU, back_set[j] ) ) ) {
				//Add this phi state to the set of states from which psi is reachable
				states = (int *) realloc(states, (reach+1)*sizeof(int));
				states[reach++] = back_set[j];
				//Mark this state as the one from which psi is reachable.
				set_bit_val(EU, back_set[j], BIT_ON);
			}
		}
	}
	free(states); states=NULL;
	return EU;
}

/**
* Solve A(phi U psi) until formula.
* @param: sparse *state_space: the state space
* @param: bitset *phi: satisfaction relation for phi formula.
* @param: bitset *psi: satisfaction relation for psi formula.
* @param: bitset *e_phi_psi: The indicator set for the formula E(Phi U Psi)
* @return: bitset *: result of A(SAT(phi) U SAT(psi)) for all states.
* NOTE: adapted for t-bounded-until from
*	  1. F. Ciesinski, M. Grober. On Probabilistic Computation Tree Logic.
*	  LNCS, Vol. 2925, Springer, pp. 147-188, 2004.
*/
bitset * get_always_until(const sparse *state_space, const bitset *phi, const bitset *psi, bitset *e_phi_psi)
{
	int i, j, k, l, size = phi->n, reach=0, ncols, *col=NULL, pcols, *back_set=NULL, *toremove=NULL, pres_size, flag;
	double val;
	//Copy the E(Phi U Psi) into the initial A(Phi U Psi) bitset
	bitset * dummy = get_new_bitset(size);
	bitset *AU = or(dummy, e_phi_psi);
	free_bitset(dummy);
	set_sparse(state_space);
	
	/* find a state in E(Phi U Psi) that does not satisfy A(Phi U Psi) */
	for(i=0;i<size;i++) {
		flag=0;
		//If it is a pure phi state
		if( get_bit_val(AU, i ) && ( ! get_bit_val(psi, i ) ) ) {
			//Take the number of non zerror elements in the i'th row
			ncols = state_space->ncols[i];
			//Take an array of non zerror elements in the i'th row
			col = state_space->val[i].col;
			//Check if we can go from the i'th state to some non psi state
			//If so then the i'th state and all of its predecessors will
			//have to be excluded from AU
			for(j=0;j<ncols;j++) {
				if( ! get_bit_val( AU, col[j] ) ) {
					flag = 1;
					break;
				}
			}
			
			if( flag ) {
				/* remove i'th state and all of its predecessors */
				pres_size = 1;
				toremove = (int *)calloc(pres_size, sizeof(int));
				toremove[0] = i;
				set_bit_val(AU, toremove[pres_size-1], BIT_OFF);
				for(k=0;k<pres_size;k++) {
					pcols = state_space->pcols[toremove[k]];
					back_set = state_space->val[toremove[k]].back_set;
					/* schedule predecessors for removal */
					for(l=0;l<pcols;l++) {
						if( get_bit_val( AU, back_set[l] ) && ( ! get_bit_val( psi, back_set[l] ) ) ) {
							toremove=(int *)realloc(toremove, (pres_size+1)*sizeof(int));
							toremove[pres_size++] = back_set[l];
							set_bit_val(AU, toremove[pres_size-1], BIT_OFF);
						}
					}
				}
				free(toremove);
				toremove = NULL;
			}
		}
	}
	return AU;
}

/**
* Universal part of PCTL and CSL unbounded until
* Solves the system of linear equations Ax=b
* @param: pM: the A matrix
* @param: pValidStates: this array contains the number of nodes as the 
* 	    first element all the other elements are the node ids, if it is
* 	    NULL then all the nodes from the A matrix are valid
* @param: pX: the initial x vector.
* 	NOTE: Is possibly freed inside by solveGaussJacobi method
* @param: pB: the b vector
* @param: BOOL revert_matrix: revert the matrix afterwards, if true
* @return: the solution of the system
*/
double * unbounded_until_universal(sparse *pM, int *pValidStates, double *pX, double *pB, BOOL revert_matrix)
{
	double err = get_error_bound();
	int max_iterations = get_max_iterations();
	
	/* get (I-P) */
	mult_mtx_cer_const(pM, -1, pValidStates);
	add_cer_cons_diagonal_mtx(pM, pValidStates, 1);
	
	/* solve (I-P)x = i_Psi */
	int method;
	if( get_method_path() == GJ) //Retrieve the l.e. solution method from the runtime.c
		method = GAUSS_JACOBI;
	else //This should be "GS" otherwise
		method = GAUSS_SEIDEL;
	
	double *pResult = solve_initial(method, pM, pX, pB, err, max_iterations, pValidStates);
	
	if(revert_matrix){
		mult_mtx_cer_const(pM, -1, pValidStates);
	}
	
	return pResult;
}

/**
* Do optimization, we exclude states from Phi which are Psi states and
* also states from which you allways go to bad i.e. not Phi ^ not Psi states
* NOTE: This optimization is applicable for until if only it has lower time bound 'subi'
* (and reward bound 'subj' if any) equal to 0. So it should be time (and reward)
* bounded until, NOT interval until, though there it may be applied in a smart way as well.
* @param phi the Phi states
* @param psi the Psi states
* @param state_space this is the sparce matrix
* @return phi_and_not_psi without phi states from which you never reach psi states 
*/
bitset * get_good_phi_states(bitset *phi, bitset *psi, sparse * state_space)
{
	bitset *not_psi = not(psi);
	bitset *eu_reach_psi_thrue_phi = get_exist_until(state_space, phi, psi);
	bitset *good_phi_states = and(eu_reach_psi_thrue_phi, not_psi);
	
	// Free memory
	free_bitset(not_psi);
	free_bitset(eu_reach_psi_thrue_phi);	
	
	return good_phi_states;
}

/**
* Make certain states (not in n_absorbing) absorbing.
* Creates a new empty sparse matrix, then assighs non absorbing rows via pointers
* and computes uniformization rates for them.
* @param: sparse *state_space: the state space
* @param: bitset *n_absorbing: not absorbing states.
* @param: double *lambda(ref.): return value of uniformizing rate(also q).
* @param: double *abse: return row_sums
* @return: sparse *: new sparse matrix with some states made absorbing.
* NOTE: makes states absorbing (in ab_state_space) by assigning pointers
*	  to rows (in state_space) only for those rows which are not to be
*	  made absorbing.
* NOTE: initially *lambda should be equal to 0!!!!
******************************************************************************/
sparse * ab_state_space(const sparse * state_space, const bitset *n_absorbing, double *lambda, double *abse, int *non_absorbing)
{
	if ( *lambda != 0.0 ) {
		printf("ERROR: lambda parameter in ab_state_space is not 0!");
		exit(1);
	}
	int i, n=n_absorbing->n;
	/*Get row sums of the initial matrix*/
	const double *e = get_row_sums();	//Note: DO NOT FREE the array you have aquired!
						//It is globally stored in runtime.c
	sparse * ab_state_space = init_matrix(state_space->rows, state_space->cols); /*Init a new matrix structure*/
	for( i=0; i<n; i++ ) { /*For all not absorbing states*/
		if( get_bit_val( n_absorbing, i ) ) {
			++*non_absorbing;
			if( ( *lambda ) < e[i] ){
				( *lambda ) = e[i]; /*Get max among available row sums*/
			}
			abse[i] = e[i];
			set_mtx_row(ab_state_space, i, state_space->ncols[i], state_space->val[i]); /*Assign rows via pointers*/
		}
	}
	//NOTE: I think we do not need aperiodicity here, since the are just absorbing and transient states!
	//It should not affect the steady state detection method since there the steady state is detected by the
	//amount of the probability mass left in the transient states. The steady state might be reached faster though,
	//because the number of self loops in the transient part of MRMC becomes smaller with this lambda.
	//
	//if( *lambda > 1.0 ) ++(*lambda); /*Make uniformization rate strictly > than max row sum*/
	return ab_state_space;
}

/**
* Free absorbing sparse matrix.
* @param		: sparse *abs: the state space
* NOTE: It's row elements were copied as structures,
*	 so the internal row (values structure) pointers refer to the original matrix
*	 and thus should not be cleaned !!! 
******************************************************************************/
void free_abs(sparse * abs)
{
	free(abs->ncols);
	free(abs->pcols);
	free(abs->val); //Do not clean each row, see the remark
	free(abs);
}
