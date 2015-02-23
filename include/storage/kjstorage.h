/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: kjstorage.h,v $
*	$Revision: 1.1 $
*	$Date: 2007/10/01 12:39:58 $
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
*	Source description: Storage for k and j vectors for unifomization_qureshi_sanders.c
*/

#include "macro.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef KJSTORAGE_H

#define KJSTORAGE_H

/*******************************************************************************
				STRUCTURE
name		: kjnode
purpose		: store a node containing k-vector, accumulated impulse rewards and the probability of the paths represented by it.
@member k	: the k-vector
@member acir	: the accumulated impulse reward
@member prob	: probability of paths represented by the k-vector and acir.
@member next	: next node in the list.
remark		:
*******************************************************************************/
typedef struct kjnode
{
	int *k;
	double acir;
	double prob;
	struct kjnode *next;	
}kjnode;


/*******************************************************************************
                                STRUCTURE
name            : kjstruc
purpose         : stores number of distict state-based reward rates+a list of kjnodes.
@member	ndsr	: number of distinct state-based reward rates
@member kjnodes	: a list of kjnodes
*******************************************************************************/
typedef struct kjstruc
{
        int ndsr;
	kjnode *kjnodes;
}kjstruc;

/*****************************************************************************
name            : get_new_kjnode
role            : create and get a new KJ-node.
@param          : int ndsr: number of distinct state-based reward rates
@return         : kjnode *: the newly created node.
remark          :
******************************************************************************/
extern kjnode *get_new_kjnode(int);

/*****************************************************************************
name            : get_new_kjstruc
role            : create and get a new KJ-struc - contains a list of KJ-nodes.
@param          : int ndsr: number of distinct state-based reward rates
@return         : kjstruc *: the newly created kjstruc.
remark          :
******************************************************************************/
extern kjstruc *get_new_kjstruc(int);

/*****************************************************************************
name            : add_new_kjnode
role            : add a new KJ node to a KJ structure.
@param          : kjstruc *kj: the KJ structure.
@param          : int *k: the new k vector.
@param          : double acir: the accumulated impulse reward on a given path.
@param          : double prob: the prob. of the path repr. by given k & acir.
remark          :
******************************************************************************/
extern void add_new_kjnode(kjstruc *, int *, double, double);

/*****************************************************************************
name            : free_kjstorage
role            : frees a given KJ-structure.
@param          : kjstruc *: the KJ structure to be freed.
remark          :
******************************************************************************/
extern void free_kjstorage(kjstruc *);

/*****************************************************************************
name            : print_kjstorage
role            : prints a given KJ-structure.
@param          : kjstruc *: the KJ structure to be printed.
remark          :
******************************************************************************/
extern void print_kjstorage(kjstruc *);

#endif

