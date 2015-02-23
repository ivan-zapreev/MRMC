/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: path_graph.h,v $
*	$Revision: 1.1 $
*	$Date: 2007/10/01 12:39:59 $
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
*	Source description: This is a simple ds for path graph gen. for DTMRMs
*	NOT SMART AT ALL - using a hashmap might be better for detatils see
*	(NOTE: slightly modified path_graph):
*		1. S. Andova, H. Hermanns and J.-P. Katoen.
*		Discrete-time rewards model-checked.
*		FORMATS 2003, LNCS, Vol. 2791, Springer, pp. 88 - 103, 2003.
*/

#include "macro.h"

#ifndef PATHGRAPH_H

#define PATHGRAPH_H

/**
* The path graph element, containes reward and corresponding probability
*/
typedef struct
{
	double reward;
	double prob;
} path_graph_ele;

/**
* The path graph node, coreesponds to one state and contains all (reward,prob) tuples,
* which identify earned reward and probability to be in this state with this reward after
* a certain amount of time.
*/
typedef struct
{
	int num;		//A number of elements in the pge array
	path_graph_ele *pge;	//(reward,prob) tuples
} path_graph;

//Create an array of length size of path graph nodes. Indexes of array correspond to the state ids
path_graph* get_new_path_graph(int size);

/**
* Insert a new (reward,prob) into a path graph node of some state
*@param pg     : a path graph pointer
*@param state  : state id
*@param reward : reward value
*@param prob   : probability value
*/
extern void insert_into_pg(path_graph *, int, double, double);

/* SLOW */
double get_prob(path_graph *, int, double);

/* For Iterations */

//Get all path graph elements for the given state
path_graph_ele * get_path_graph_ele(path_graph *, int);
//Get the number of path graph elements for the given state
int get_path_graph_num(path_graph *, int);
//Delete a path graph element l for the given state
int delete_from_pg(path_graph *pg, int state, int l);

//Free all allocated memory
void free_path_graph(path_graph *, int);

/* For Test Only */
void print_path_graph(path_graph *, int);

#endif
