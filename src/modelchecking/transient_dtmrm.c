/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: transient_dtmrm.c,v $
*	$Revision: 1.7 $
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
*	Source description: Perform Transient Analysis for PRCTL - X, U.
*/

#include "transient_dtmrm.h"

/**
* Solve the N-J-bounded until operator for DTMRM.
* @param: bitset *phi: SAT(phi).
* @param: bitset *psi: SAT(psi).
* @param: double supi: sup I, I is a time interval
* @return: double *: result of the N-J-bounded until operator for all states.
* NOTE: see (slightly modified path_graph):
*	1. S. Andova, H. Hermanns and J.-P. Katoen.
*	Discrete-time rewards model-checked.
*	FORMATS 2003, LNCS, Vol. 2791, Springer, pp. 88 - 103, 2003.
*/
static double * dtmrm_bounded_until_universal(bitset *phi, bitset *psi, double subi, double supi, double subj, double supj)
{
	sparse *pM = get_state_space();
	
	//WARNING: The common get_good_phi_states optimization can not
	//be employed, because we may have a case with subi > 0 ( and subj > 0)
	//So all the optimization for subi == 0 & subj == 0 are done before
	
	//Translate phi and psi bitsets into array of node ids,
	//where first elements defines the number of ids
	int *phi_set = count_set(phi), *psi_set = count_set(psi);
	
	const int size = phi->n, phi_size = phi_set[0], psi_size = psi_set[0];
	int i, j, k, l, state, col, count, found, num;
	
	double val, *reward = getStateRewards();
	
	path_graph *pg_n=NULL;       //a path graph of the last iteration
	path_graph *pg_n_plus=NULL;  //a new iteration path graph
	path_graph_ele *pge, *tpge;
	
	double *result=(double *)calloc(size, sizeof(double)), prob=0.0, value;
	
	//For every states, taken as initial state, compute probabilities
	for( i=0; i < size; i++ )
	{
		//First check whether we are already in a psi state if yes and
		//if 0 time and reward is allowed then according to the theory
		//we assign porob to 1 and skip the rest
		//Although it looks ugly this is not an error, see the theory
		if( get_bit_val( psi, i ) &&
		    ( subi <= 0 ) && ( supi >= 0 ) &&
		    ( subj <= 0 ) && ( supj >= 0 ) )
		{
			result[i]=1.0;
			continue;
		}
		
		//Get an initial path graph, insert initial node for the current state and time 0
		pg_n = get_new_path_graph(size);
		insert_into_pg(pg_n, i, 0.0, 1.0);
		
		//Building the path graphs for all times in the time interval [subi,supi]
		for(j=0;j<supi;j++)
		{
			//Allocate path graph for time (j+1)
			pg_n_plus = get_new_path_graph(size);
			//For all phi states 
			for(k=1;k<=phi_size;k++)
			{
				state=phi_set[k];
				num = get_path_graph_num(pg_n, state);
				pge = get_path_graph_ele(pg_n, state);
				//Check if this phi state was reached at time j
				if(num>0)
				{
					//If YES then try to extend the corresponding path
					count=0;
					do
					{
						//Try non self loops first
						found=get_mtx_next_val(pM, state, &col, &val, &count);
						if(!found) break;
						//Check if the transition goes to a phi or a psi state
						if( get_bit_val( phi, col ) || get_bit_val( psi, col ) )
						{
							//Extend all paths ending in state at time j with this transition
							tpge=pge;
							for(l=0;l<num;l++,tpge++)
								insert_into_pg(pg_n_plus, col, reward[state]+tpge->reward,val*tpge->prob);
						}
					}while(found);
					found=get_mtx_diag_val(pM, state, &value);
					//If there is a self loop then it goes to phi or psi state :)
					if(found)
					{
						//Extend all paths ending in state at time j with this transition
						tpge=pge;
						for(l=0;l<num;l++,tpge++)
							insert_into_pg(pg_n_plus, state, reward[state]+tpge->reward,value*tpge->prob);
					}
				}
			}
			
			//If the time is right i.e. we are withing [subi,supi] then check for reached psi states!
			if( (j+1) >= subi && (j+1) <= supi )
			{
				//for all psi states
				for( k=1; k <= psi_size; k++ )
				{
					state=psi_set[k];
					num = get_path_graph_num(pg_n_plus, state);
					pge = get_path_graph_ele(pg_n_plus, state);
					//If a psi state was reached by some paths then
					//check all rewards with which we reached it...
					for( l=0; l < num;)
					{
						//If reward is within the required interval i.e. [subj,supj]
						if( ( pge->reward >= subj ) && ( pge->reward <= supj ) )
						{
							//accumulate the probability to get this reward and reach psi state
							prob+=pge->prob;
							//According to the Andova algorithms we do not extend
							//this path anymore, so we remove it from the path graph.
							num=delete_from_pg(pg_n_plus, state, l);
						}
						else {l++;pge++;}
					}
				}
			}
			free_path_graph(pg_n, size);
			pg_n = pg_n_plus;
			pg_n_plus=NULL;
		}
		result[i]=prob; prob=0.0;
		free_path_graph(pg_n, size);
	}

	free(psi_set);
	free(phi_set);
	return result;
}

/**
* Solve the N-J-bounded until operator for DTMRM.
* Adds optimization for the case subi == 0 & subj == 0
* and then calls dtmrm_bounded_until(...)
* @param: bitset *phi: SAT(phi).
* @param: bitset *psi: SAT(psi).
* @param: double supi: sup I, I is a time interval
* @return: double *: result of the N-J-bounded until operator for all states.
* NOTE: see (slightly modified path_graph):
*	1. S. Andova, H. Hermanns and J.-P. Katoen.
*	Discrete-time rewards model-checked.
*	FORMATS 2003, LNCS, Vol. 2791, Springer, pp. 88 - 103, 2003.
*/
double * dtmrm_bounded_until(bitset *phi, bitset *psi, double subi, double supi, double subj, double supj)
{
	double *result = NULL;
	if( subi == 0 && subj == 0 ){
		//Do optimization, we exclude states from Phi which are Psi states and
		//also states from which you allways go to bad i.e. not Phi ^ not Psi states
		//This is only possible if until has lower time bound 'subi'
		//(and reward bound 'subj' if any) equal to 0.
		bitset *good_phi_states = get_good_phi_states( phi, psi, get_state_space() );
		
		result = dtmrm_bounded_until_universal(good_phi_states, psi, subi, supi, subj, supj);
		
		free_bitset(good_phi_states);
	}else{
		result = dtmrm_bounded_until_universal(phi, psi, subi, supi, subj, supj);
	}
	return result;
}

/**
* Solve the N-J-bounded until operator for DTMRM with formula dependent lumping.
* Adds optimization for the case subi == 0 & subj == 0
* NOTE: NO support for impulse rewards!!!
* @param: bitset *phi: SAT(phi).
* @param: bitset *psi: SAT(psi).
* @param: double supi: sup I, I is a time interval
* @return: double *: result of the N-J-bounded until operator for all states.
* NOTE: see (slightly modified path_graph):
*	1. S. Andova, H. Hermanns and J.-P. Katoen.
*	Discrete-time rewards model-checked.
*	FORMATS 2003, LNCS, Vol. 2791, Springer, pp. 88 - 103, 2003.
*/
double * dtmrm_bounded_until_lumping(bitset *phi, bitset *psi, double subi, double supi, double subj, double supj){
	sparse *original_state_space = get_state_space();
	double *result, *lumped_result;
	bitset *good_phi_states = NULL;
	
	//WARNING: The common get_good_phi_states optimization can not
	//be employed, because we may have a case with subi > 0 ( and subj > 0)
	//Add optimization for the case subi == 0 & subj == 0
	if( subi == 0 && subj == 0 ){
		good_phi_states = get_good_phi_states( phi, psi, original_state_space );
	}
	
	//Create initial partition for bounded until formula and lump
	partition *P = init_partition_formula( ( good_phi_states != NULL ? good_phi_states : phi ) , psi, FALSE);
	sparse *Q = lump(P, original_state_space);
	
	//Free the row sums vector because otherwise it will be lost
	free_row_sums();
	//Set lumped CTMC to be the state space, NOTE: resets the row_sums!
	set_state_space(Q);
	
	//Lump the bitsets
	bitset *lumped_phi = lump_bitset(P, ( good_phi_states != NULL ? good_phi_states : phi ) );
	bitset *lumped_psi = lump_bitset(P, psi);
	
	//Change state rewards
	double * p_old_rew, * p_new_rew;
	p_old_rew = getStateRewards();
	p_new_rew = change_state_rewards(p_old_rew, P);
	setStateRewards(p_new_rew);
	
	lumped_result = dtmrm_bounded_until(lumped_phi, lumped_psi, subi, supi, subj, supj);
	
	//Unlump the resulting vector
	result = unlump_vector(P, original_state_space->rows, lumped_result);
	
	//Restore state rewards
	freeStateRewards();
	setStateRewards(p_old_rew);
	
	//Free allocated memory
	free_bitset(lumped_phi);
	if ( good_phi_states != NULL ){
		free_bitset( good_phi_states );
	}
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
