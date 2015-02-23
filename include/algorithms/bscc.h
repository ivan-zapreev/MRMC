/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: bscc.h,v $
*	$Revision: 1.1 $
*	$Date: 2007/10/01 12:39:49 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
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
*	Source description: This file contains functions' defenitions for the BSCCs detection.
* 	   The algorithm is based on the Tarjan's algorithm for searching SCCs.
*/

#include "macro.h"

#include "bitset.h"
#include "sparse.h"
#include "stack.h"
#include "runtime.h"

#ifndef BSCC_H

#define BSCC_H

/**
* This structure will hold all required BSCC search data.
*/
typedef struct bsccs_struct{
  /*The state space we are using*/
  sparse * pStateSpace;
  
  /*Store the state space dimension, dublicates pStateSpace->row*/
  int size;

  /*This bitset stores the visited states;*/
  bitset *pVisitedStates;

  /*This bitset stores states that belong to some component or from which we can reach a component;*/
  bitset *pInComponentStates;

  /*This array stores mapping from node ids to the BSCC ids The index of the array is
    the index of node and value in the corresponding Cell is the id of the BSCC to which
	the given node belongs. If it is 0 then the BSCC was not found yet or it doesn't
	belong to any BSCC*/
  int * pBSCCs;

  /*This variable stores the DFS order*/
  int dfs_order;

  /*This variable stores the id of the lately found bscc (Supports up to 255 bsccs)*/
  int bscc_counter;
} TBSCCs;

/**
* This function simply instantiates and returns the pointer to TBSCCs struct
* @param pStateSpaceTmp a state space to work with
* @return an empty TBSCCs structure
*/
extern TBSCCs * allocateTBSCCs(sparse * pStateSpaceTmp);

/**
* This function returns the list of ids for a newly found BSCCs
* @param pBSCCsHolder the work structure to store all required data for
*               BSCCs search for a particular state space
* @param pStates the set of states
* @return the two dimensional array (result[][])
* result[0] the number of newly found BSCCs
* result[1][0] the id of a newly found bsccs,
* result[1][1] the number of elements in a bscc
* result[1][2] the id of the node if result[1][1] = 1
*/
extern int ** getNewBSCCs(TBSCCs *pBSCCsHolder, bitset *pStates);

/**
* This function is used to return the number of BSCCs known in _ppBSCCs.
* This number is stored in the 0 element of the ppNewBSCCs array, so it
* is a pointer (int *) which is used to satore an integer value only.
* @param _ppBSCCs the structure containing BSCCs along
* with the number of them, which is stored in _ppBSCCs[0]
* @return the number of BSCCs newly found (_ppBSCCs[0])
**/
inline int getBSCCCount(int ** _ppBSCCs);

/**
* This method returns a mapping array from states into the BSCCs' ids.
* @param pBSCCsHolder the structure to retrieve BSCCs mapping from
* @return The array which stores mapping from node ids into the BSCC ids
* The index of the array is the index of node and value in the corresponding
* Cell is the id of the BSCC to which the given node belongs
* If it is 0 then the BSCC was not found yet or it doesn't belong to any BSCC
*/
extern int * getStatesBSCCsMapping(TBSCCs * pBSCCsHolder);

/**
* This method is used to deallocate TBSCCs and all associated data.
* pBSCCsHolder the pointer to the structure to be cleaned
*/
extern void freeTBSCC(TBSCCs * pBSCCsHolder);

/*
* This method is used to free memory used for storing BSCCs
* @param ppLastBSCCs the set of BSCCs returned by the
*                   int ** getNewBSCCs(bitset *)
*                   function.
*/
extern void freeBSCCs(int ** ppLastBSCCs);

/**
* @param pBSCCsHolder the structure to retrieve bscc_counter from
* @return the number of BSCCs found so far.
*/
extern int getBSCCCounter(TBSCCs * pBSCCsHolder);

#endif

