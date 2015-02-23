/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: kjstorage.c,v $
*	$Revision: 1.3 $
*	$Date: 2007/05/03 13:37:57 $
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
*	Source description: Storage for k and j vectors for unifomization_quresanders.c
*/

#include "kjstorage.h"

/*****************************************************************************
name            : get_new_kjnode
role            : create and get a new KJ-node.
@param          : int ndsr: number of distinct state-based reward rates
@return         : kjnode *: the newly created node.
remark          : 
******************************************************************************/
kjnode *get_new_kjnode(int ndsr)
{
	kjnode *new_node = (kjnode *)calloc(1, sizeof(kjnode));
	new_node->k = (int *)calloc(ndsr, sizeof(int));
	new_node->prob = 0.0;
	new_node->acir = 0.0;
	new_node->next = NULL;
	return new_node;
}

/*****************************************************************************
name            : get_new_kjstruc
role            : create and get a new KJ-struc - contains a list of KJ-nodes.
@param          : int ndsr: number of distinct state-based reward rates
@return         : kjstruc *: the newly created kjstruc.
remark          :
******************************************************************************/
kjstruc *get_new_kjstruc(int ndsr)
{
	kjstruc *new_kjstruc=(kjstruc *)calloc(1, sizeof(kjstruc));
	new_kjstruc->ndsr = ndsr;
	new_kjstruc->kjnodes = NULL;
	return new_kjstruc;
}

/*****************************************************************************
name            : add_new_kjnode
role            : add a new KJ node to a KJ structure.
@param          : kjstruc *kj: the KJ structure.
@param		: int *k: the new k vector, its values are just copyed here.
@param		: double acir: the accumulated impulse reward on a given path.
@param		: double prob: the prob. of the path repr. by given k & acir.
remark          :
******************************************************************************/
void add_new_kjnode(kjstruc *kj, int *k, double acir, double prob)
{
	int ndsr = kj->ndsr, i, found;
	kjnode *kjnodep=kj->kjnodes, *pkjnode=kjnodep;

	/* Search for the same reward value in a list.
           If one found and it has the same k array then
           add probability*/

	while(kjnodep!=NULL)
	{
		if(kjnodep->acir==acir)
		{
			found=1;
			for(i=0;i<ndsr;i++)
				if(k[i]!=kjnodep->k[i])
					found=0;
			/* completely matched */
			if(found==1)
			{
				kjnodep->prob+=prob;
				return; //The job is done
			}
		}
		pkjnode=kjnodep;
		kjnodep=kjnodep->next;
	}

        /*If the corresponding reward was not found or if the
          k array is different for it then add a new entry*/

	if(kj->kjnodes==NULL)
	{
		kj->kjnodes = get_new_kjnode(ndsr);
		//Copy the array data
		memcpy(kj->kjnodes->k, k, ndsr * sizeof(int) );
		kj->kjnodes->acir=acir;
		kj->kjnodes->prob=prob;
	}
        else 
        {
                pkjnode->next = get_new_kjnode(ndsr);
		//Copy the array data
		memcpy(pkjnode->next->k, k, ndsr * sizeof(int) );
		pkjnode->next->acir=acir;
                pkjnode->next->prob=prob;
        }
}

/*****************************************************************************
name            : free_kjstorage
role            : frees a given KJ-structure.
@param          : kjstruc *: the KJ structure to be freed.
remark          :
******************************************************************************/
void free_kjstorage(kjstruc *kj)
{
	kjnode *kjn = kj->kjnodes, *to_free;
	while(kjn!=NULL)	
	{
		to_free=kjn;
		kjn=kjn->next;
		free(to_free->k);
		free(to_free);
	}
	kj->kjnodes=NULL;
	free(kj);
}

/*****************************************************************************
name            : print_kjstorage
role            : prints a given KJ-structure.
@param          : kjstruc *: the KJ structure to be printed.
remark          :
******************************************************************************/
void print_kjstorage(kjstruc *kj)
{
	int i, ndsr = kj->ndsr;
 	kjnode *kjn = kj->kjnodes;
	while(kjn!=NULL)	
	{
		printf("{p=%e, acir=%e, k=[", kjn->prob, kjn->acir);
		for(i=0;i<ndsr;i++)
			printf("%d, ", kjn->k[i]);
		printf("]}\n");
		kjn=kjn->next;
	}
}

