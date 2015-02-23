/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: path_graph.c,v $
*	$Revision: 1.2 $
*	$Date: 2007/05/03 13:37:58 $
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

#include "path_graph.h"

//Create an array of length size of path graph nodes. Indexes of array correspond to the state ids
path_graph* get_new_path_graph(int size)
{
	int i;
	path_graph* pg;
	pg = (path_graph*)calloc(size,sizeof(path_graph));
	for(i=0;i<size;i++)
		{pg[i].num=0;pg[i].pge=NULL;}
	return pg;
}

/**
* Insert a new (reward,prob) into a path graph node of some state
*@param pg     : a path graph pointer
*@param state  : state id
*@param reward : reward value
*@param prob   : probability value
*/
void insert_into_pg(path_graph *pg, int state, double reward, double prob)
{
	int i, num = pg[state].num, found=0;
	path_graph_ele *trav=pg[state].pge;

	/*First for a given state look for the reward
          if there is such then increase probability*/
	for(i=0; i<num; i++, trav++)
	{
		//If the state has this reward then just increase the probability
		if(trav->reward==reward)
		{
			trav->prob+=prob;
			found=1;
		}
	}
	//if there was no such reward then insert a new path graph element
	if(found==0)
	{
		++pg[state].num;
		pg[state].pge=(path_graph_ele *)realloc(pg[state].pge, pg[state].num*sizeof(path_graph_ele));
		trav=pg[state].pge;
		trav[pg[state].num-1].reward=reward;
		trav[pg[state].num-1].prob=prob;
	}
}

//Delete a path graph element l for the given state
int delete_from_pg(path_graph *pg, int state, int l)
{
	int i, num = pg[state].num;
	path_graph_ele *trav=pg[state].pge;
	if(l<num)
	{
		//decrease the number of elements
		pg[state].num=pg[state].num-1;
		num=pg[state].num;
		//shift elements to the left
		for(;l<num;l++)
			trav[l]=trav[l+1];
		//reallocate memory
		if(num>0)
			pg[state].pge=(path_graph_ele *)realloc(pg[state].pge, num*sizeof(path_graph_ele));
		else
		{
			free(pg[state].pge);
			pg[state].pge=NULL;
		}
	}
	return num;
}

/* SLOW */
double get_prob(path_graph *pg, int state, double reward)
{
	int i, num = pg[state].num;
	double ret_val=0.0;
	path_graph_ele *trav=pg[state].pge;
	for(i=0; i<num; i++, trav++)
		if(trav->reward==reward)
		{
			ret_val=trav->prob;
			break;	//there can be only one such reward in trav 
		}
	return ret_val;
}

/* For Iterations */

//Get all path graph elements for the given state
path_graph_ele * get_path_graph_ele(path_graph *pg, int state)
{
	return pg[state].pge;
}

//Get the number of path graph elements for the given state
int get_path_graph_num(path_graph *pg, int state)
{
	return pg[state].num;
}

//Free all allocated memory
void free_path_graph(path_graph *pg, int size)
{
	int i;
	if(pg)
	{
		for(i=0;i<size;i++)
			if(pg[i].pge)
			{
				free(pg[i].pge);
				pg[i].pge=NULL;
			}
		free(pg);
	}
}

/* For Test Only */
void print_path_graph(path_graph *pg, int size)
{
	int i, j, num;
	path_graph_ele *trav=NULL;
	for(i=0;i<size;i++)
	{
		trav = pg[i].pge;
		num = pg[i].num;
		printf("For State %d\n", i);
		printf("(R,P)s = {");
		for(j=0;j<num;j++,trav++)
			printf("(%e,%e),",trav->reward,trav->prob);
		printf("}\n");
	}
}

