/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: prctl.c,v $
*	$Revision: 1.6 $
*	$Date: 2007/09/30 13:07:48 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Maneesh Khattri
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
*	Source description: Solve formulas in PRCTL. 
*/

#include "prctl.h"

/*****************************************************************************
name		: ef_bounded: E[n}[subJ, supJ][PHI]
role       	: solve N-R bounded E formula in DMRM
@param		: int steps: n.
@param		: bitset *phi: satisfaction relation for phi formula.
@return		: double *: result of N-R bounded E formula in DMRM.
remark 	        : 
******************************************************************************/
static double* ef_bounded(int steps, bitset *phi)
{
	sparse * state_space = get_state_space();
	int i, j, size = state_space->rows;
	double *result=(double *)calloc(size, sizeof(double)), *temp_result, *rewards = getStateRewards();
	bitset *tt=get_new_bitset(size), *psi=get_new_bitset(size);
	fill_bitset_one(tt);
	
	for(i=0;i<size;i++) {
		if( get_bit_val( phi, i ) ) {
			set_bit_val(psi, i, BIT_ON);
			temp_result=until( TIME_INTERVAL_FORM, tt, psi, 0.0, steps, FALSE);
			for(j=0;j<size;j++) {
				result[j]+=temp_result[j]*rewards[i];
			}
			free(temp_result);
			set_bit_val(psi, i, BIT_OFF);
		}
	}
	for(i=0;i<size;i++) {
		result[i]/=(steps+1);
	}
	free_bitset(psi);
	free_bitset(tt);
	return result;
}

/*****************************************************************************
name		: ef_unbounded: E[subJ, supJ][PHI]
role       	: solve unbounded E formula in DMRM
@param		: bitset *phi: satisfaction relation for phi formula.
@return		: double *: result of N-R bounded E formula in DMRM.
remark 	        : 
******************************************************************************/
static double* ef_unbounded(bitset *phi)
{
	sparse * state_space = get_state_space();
	int i, j, size = state_space->rows;
	double *result=(double *)calloc(size, sizeof(double)), *temp_result, *rewards = getStateRewards();
	bitset *tt=get_new_bitset(size), *psi=get_new_bitset(size);
	fill_bitset_one(tt);
	
	for(i=0;i<size;i++) {
		if( get_bit_val( phi, i ) ) {
			set_bit_val(psi, i, BIT_ON);
			temp_result=until( TIME_UNBOUNDED_FORM, tt, psi, 0.0, 0.0, FALSE);
			for(j=0;j<size;j++){
				result[j]+=temp_result[j]*rewards[i];
			}
			free(temp_result);
			set_bit_val(psi, i, BIT_OFF);
		}
	}
	free_bitset(psi);
	free_bitset(tt);
	return result;
}

/*****************************************************************************
name		: ef
role       	: solve E formula in DMRM
@param		: int steps: n.
@param		: bitset *phi: satisfaction relation for phi formula.
@return		: double *: result of N-R bounded E formula in DMRM.
remark 	        : 
******************************************************************************/
double* ef(int steps, bitset *phi)
{
	sparse * state_space = get_state_space();
	int size = state_space->rows;
	double *result;
	if( isRunMode(PRCTL_MODE) )
	{
		if(steps==0)
			result=ef_unbounded(phi);
		else if(steps!=0)
			result=ef_bounded(steps, phi);
	}
	else
	{
		printf("ERROR: Unsupported mode for E formula\n");
		exit(1);
	}
	return result;
}

/*****************************************************************************
name		: cf
role       	: solve C formula in DMRM
@param		: int steps: n.
@param		: bitset *phi: satisfaction relation for phi formula.
@return		: double *: result of N-R bounded C formula in DMRM.
remark 	        : 
******************************************************************************/
double* cf(int steps, bitset *phi)
{
	sparse * state_space = get_state_space();
	int size = state_space->rows,i, j;
	double *result=(double *)calloc(state_space->rows, sizeof (double)), *rewards = getStateRewards();
	double *result_1 = (double *)calloc(state_space->rows, sizeof (double));
	double *result_2 = (double *)calloc(size, sizeof (double)), * pTmp = NULL;
	
	if( isRunMode(PRCTL_MODE) ) {
		for(i=0;i<size;i++) {
			if( get_bit_val( phi, i ) ) {
				bzero(result_1, state_space->rows*sizeof (double));
				result_1[i] = 1.0;
				/*Compute P^supi*i_phi*/
				for(j = 0; j < steps ; j++) {
					multiply_mtx_MV( state_space, result_1, result_2);
					pTmp = result_1;
					result_1 = result_2;
					result_2 = pTmp;
				}
				for(j=0;j<size;j++) {
					result[j]+=result_1[j]*rewards[i];
				}
			}
		}
	} else {
		printf("ERROR: Unsupported mode for C formula\n");
		exit(1);
	}
	free(result_1);
	free(result_2);
	return result;
}

/*****************************************************************************
name		: yf
role       	: solve Y formula in DMRM
@param		: int steps: n.
@param		: bitset *phi: satisfaction relation for phi formula.
@return		: double *: result of N-R bounded C formula in DMRM.
remark 	        :
******************************************************************************/
double* yf(int steps, bitset *phi)
{
	sparse * state_space = get_state_space();
	int size = state_space->rows,i, j;
	double *rewards = getStateRewards();
	double *result1=(double *)calloc(state_space->rows, sizeof (double));
	double *result2=(double *)calloc(state_space->rows, sizeof (double));
	double *rewards2 = (double *)calloc(size, sizeof(double));
	
	for(i=0;i<size;i++) {
		if( get_bit_val( phi, i ) ){
			rewards2[i] = rewards[i];
		}
	}
	if( isRunMode(PRCTL_MODE) ) {
		for(i=0;i<steps;i++) {
			multiply_mtx_MV( state_space, result1, result2);
			for(j=0;j<size;j++) {
				result1[j] = rewards2[j]+result2[j];
			}
		}
	} else {
		printf("ERROR: Unsupported mode for Y formula\n");
		exit(1);
	}
	
	free(result2);
	free(rewards2);
	return result1;
}

