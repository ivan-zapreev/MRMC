/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: lump.h,v $
*	$Revision: 1.1 $
*	$Date: 2007/10/01 12:39:54 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Tim Kemna, Ivan S Zapreev
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
*	Source description: Lumping of Markov chains.
*	Uses: DEF: lump.h, partition.h, splay.h, sparse.h, bitset.h, label.h, runtime.h, macro.h, 
*		LIB: partition.c, splay.c, sparse.c, bitset.c, label.c, 
*/


#ifndef LUMP_H
#define LUMP_H

#include "label.h"
#include "sparse.h"
#include "splay.h"

/**
* This method returns an initial partition based on the labelling structure.
* The initial partition contains one block for each combination of labels.
* @param	: labelling *labellin: The labelling structure.
* @return	: The initial partition.
*/
partition *init_partition(labelling *labellin);

/**
* This method returns a formula-dependent initial partition.
* @param	: bitset *phi: satisfaction relation for phi formula.
* @param	: bitset *psi: satisfaction relation for psi formula.
* @param	: BOOL interval: CSL interval until formula, if true
*		PCTL/CSL (un)bounded until formula, otherwise
* @return	: The initial partition.
*	{Sat(psi), Sat(phi && !psi)), Sat(!psi && !psi)} for (un)bounded until formulas.
*	{Sat(phi && !psi), Sat(phi && psi), Sat(!phi && psi)), Sat(!psi && !psi)} for interval until formulas.
*/
partition *init_partition_formula(const bitset *phi, const bitset* psi, const BOOL interval);

/**
* This method returns an linked-list of pointers to blocks, one for each block in P.
* @param		: partition *P: The initial partition.
* @return	: The linked-list of pointers to blocks.
*/
block_pointer *init_splitters(partition *P);

/**
* This method computes the optimal lumped matrix of the input matrix.
* @param	: partition *P: The initial partition.
* @param	: sparse *Q: The matrix to lump.
* @return	: The lumped matrix.
*/
sparse *lump(partition *P, sparse *Q);

/**
* This method changes the labelling structure such that it corresponds to the partition.
* @param	: labelling *labellin: the labelling structure.
* @param	: partition *P: the partition.
*/
void change_labelling(labelling *labellin, partition *P);

/**
* This method changes the state rewards structure such that it
* corresponds to the partition. The actual change is done only
* in the case when the provided p_old_rew pointer is not NULL.
* NOTE: 	We do not free the old_rewards structure because
*		it should be done on the outer level.
* @param	: double * p_old_rew: the the rewards structure.
* @param	: partition *P: the partition.
* @return 	: The rewards structure after lumping or NULL if the original
*		  rewards structure is NULL
*/
double * change_state_rewards(double * p_old_rew, partition *P);

/**
* Unlump a probability vector with respect to the partition.
* @param	: partition *P: the partition of the original state space.
* @param	: int size: the number of states in the original state space.
* @param	: double *result_lumped: the probability vector of the lumped state space.
* @return	: the probability vector of the original state space.
*/
double *unlump_vector(partition *P, int size, double *result_lumped);

/**
* Lump a bitset with respect to the partition.
* @param	: partition *P: the partition of the original state space.
* @param	: bitset *phi: the original bitset.
* @return	: the lumped bitset.
*/
bitset *lump_bitset(partition *P, bitset *phi);

#endif
