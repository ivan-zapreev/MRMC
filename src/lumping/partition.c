/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: partition.c,v $
*	$Revision: 1.6 $
*	$Date: 2007/10/14 15:11:58 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Tim Kemna, Ivan S. Zapreev
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
*	Source description: Manage partition structure for lumping.
*	Uses: DEF: partition.h
*/

#include <stdio.h>
#include <stdlib.h>
#include "macro.h"
#include "partition.h"


/**
* This method is used for creating a new Reward_Tree node with the given reward rate.
* @param reward	: The reward rate
* @return the newly created node
*/
inline Reward_Tree * createNewRewardNode(const double reward){
	Reward_Tree * reward_node = (Reward_Tree *)  calloc(1, sizeof(Reward_Tree));
	reward_node->reward = reward;
	reward_node->left = reward_node->right = NULL;
	reward_node->subblock = NULL;

	return reward_node;
}

/**
* This is a RECUIRSIVE procedure that is used for searching an appropriate reward node,
* and if there is no such node yet, it creates it.
* @param reward_root	: The reward tree we have to find/insert a node for the given reward
* @param reward		: The reward we are looking for
* @return the found of newly created Reward_Tree node for the given reward 
*/
Reward_Tree * getStateRewardNode(Reward_Tree * reward_root, const double reward){
	Reward_Tree * resulting_node = NULL;
	if( reward_root->reward == reward){
		//In this case there is already a node with the required reward
		resulting_node = reward_root;
	} else if(reward_root->reward > reward){
		//If the reward is smaller
		if( reward_root->left != NULL ){
			//Go further
			resulting_node = getStateRewardNode(reward_root->left, reward);
		}else{
			//We have to construct a new node;
			resulting_node = reward_root->left = createNewRewardNode(reward);
		}
	} else {
		//If the reward is greater
		if( reward_root->right != NULL ){
			//Go further
			resulting_node = getStateRewardNode(reward_root->right, reward);
		}else{
			//We have to construct a new node;
			resulting_node = reward_root->right = createNewRewardNode(reward);
		}
	}

	return resulting_node;
}

/**
* This method is used to recursively free the reward tree
* NOTE: The blocks placed in the tree node are left allocated,
*	  because they are expected to be referenced from somewhere
*	  else.
* @param retard_tree	: the reward tree to be deleted
*/
void free_reward_tree(Reward_Tree * reward_tree){
	if( reward_tree != NULL){
		if(reward_tree->left != NULL){
			free_reward_tree(reward_tree->left);
		}
		if(reward_tree->right != NULL){
			free_reward_tree(reward_tree->right);
		}
		
		free(reward_tree);
	}
}

/*****************************************************************************
name		: add_state
role       	: This method adds a state to a block.
@param     	: block *b: The block.
@param     	: state *s: The state to add.
******************************************************************************/
inline void add_state(block *b, state *s){
	s->prev = NULL;
	if(b->num_states == 0){
		s->next = NULL;
	}else{
		s->next = b->states;
		b->states->prev = s;
	}
	b->num_states++;
	
	// update block of the state
	s->b = b;
	
	b->states = s;
}

/*****************************************************************************
name		: delete_state
role       	: This method deletes a state from a block.
@param     	: partition *P: The current partition.
@param     	: block *b: The block.
@param     	: int index: The index of the state to add.
******************************************************************************/
inline state *delete_state(partition *P, block *b, int index){
	state *s;
	
	s = P->states[index];
	if(b->states == s){
		b->states = b->states->next;
		if(b->states != NULL){
			b->states->prev = NULL;
		}
	}else{
		if(s->next != NULL){
			s->next->prev = s->prev;
		}
		s->prev->next = s->next;
	}
	
	b->num_states--;
		
	return s;
}

/*****************************************************************************
name		: init_block
role       	: This method creates and initialises an empty block.
@return   	: The block.
******************************************************************************/
inline block *init_block(){
	block *b;
	
	b = (block *) calloc(1, sizeof(block));
	if(b == NULL){
		printf("ERROR: Allocation of storage for failed partition.c::init_block(...)\n");
		exit(1);
	}
	b->states = NULL;
	b->tree = NULL;
	b->num_states = 0;
	b->flags = 0;
	
	return b;
}

/**
* Here we get the original state-space size,
* that has changed because of lumping.
* @return the original state-space size
*/
inline int get_unlumped_state_space_size(partition * P){
	int result = 0;
	block *bl;
	for(bl = P->blocks; bl != NULL; bl = bl->next){
		result += bl->num_states;
	}
	return result;
}

/**
* This method is used to retrieve the state index in the lumped state space.
* @param pPartition the state-space partition
* @param state_idx the state index in the unlumped state space
* @return "-1" if "state_idx" is not within the state-index range of
*	the original state space, and the state index in the lumped space otherwise.
*/
inline int getLumpedStateIndex(partition *pPartition, const int state_idx){
	IF_SAFETY( pPartition != NULL )
		const int ns = get_original_state_space_size( pPartition );
		if( ( state_idx >= 0 ) && ( state_idx < ns ) ){
				return block_of( pPartition, state_idx )->row;
		} else {
			return -1;
		}
	ELSE_SAFETY
		printf("ERROR: Can not retrieve the lumped-state index the partition is NULL.\n");
		exit(1);
	ENDIF_SAFETY
}

/**
* This method prints the position of the bits in the original
* bitset whose value is 1.
* @param P: The partition.
* @param b: The bitset corresponding to the lumped state space.
*/
void print_original_states(partition *P, bitset *b){
	const int ns = get_original_state_space_size(P);
	int xj;
	BOOL bFirst = TRUE;
	if( b != NULL ){
		printf("{ ");
		for(xj = 0; xj < ns; xj++){
			if( get_bit_val( b, block_of(P,xj)->row ) ){
				if( ! bFirst ){
					printf(", ");
				}else{
					bFirst = FALSE;
				}
				printf("%d", xj+1);
			}
		}
		printf(" }");
	}else{
		printf("??\n");
		printf("WARNING: Trying to print an element of a non-existing bitset.");
	}
}

/**
* This method prints TRUE if index is in the unlumped bitset 'b',
* otherwise false.
* @param P: The partition.
* @param b: The bitset corresponding to the lumped state space.
* @param const: The original-state-space state index
*/
void print_original_state( const partition *P, const bitset *b, const int index ){
	if( b != NULL){
		if( get_bit_val(b, block_of(P,index)->row ) ){
			printf("TRUE");
		}else{
			printf("FALSE");
		}
	}else{
		printf("??\n");
		printf("WARNING: Trying to print an element of a non-existing bitset.");
	}
}

/**
* Print the unlumped vector of probabilities.
* @param P: The partition.
* @param length: The length of the 'vec' array.
* @param vec: The array of probabilities to be printed.
*/
void print_state_probs_partition(partition *P , int length, double * vec)
{
	int j, i;
	char buffer[255];
	const int n = get_original_state_space_size(P);
	
	//Get the printing pattern that agrees with the error bound precision
	sprintf(buffer, "%%1.%dlf", get_error_bound_precision());
	
	if( vec != NULL ){
		printf("( ");
		for( j = 0 ; j < n; j++ )
		{
			i = block_of(P,j)->row;
			
			printf( buffer, vec[i] );
			
			if (j != n-1) printf(", ");
		}
		printf(" )");
	}else{
		printf("NULL");
	}
}

/**
* This method prints the probability of one state only, with
* the unlumping done first.
* @param P: The partition.
* @param index: The state index in the original state space.
* @param length: The length of the 'vec' array.
* @param vec: The array of probabilities to be printed.
*/
void print_state_prob_partition( const partition *P , const int index, const int length, const double * vec){
	int lumped_index;
	char buffer[255];
	const int n = get_original_state_space_size(P);
	if( vec != NULL ){
		//Get the printing pattern that agrees with the error bound precision
		sprintf(buffer, "%%1.%dlf", get_error_bound_precision());
		
		lumped_index = block_of(P,index)->row;
		printf( buffer, vec[lumped_index] );
	}else{
		printf("??\n");
		printf("WARNING: Trying to print an element of a non-existing array.");
	}
}

/*****************************************************************************
name		: free_partition
role       	: This method frees the partition.
@param     	: partition *P: The partition.
******************************************************************************/
void free_partition(partition *P){
	block *B, *next;
	
	B = P->blocks;
	while(B != NULL){
		next = B->next;
		free_block(B);
		B = next;
	}
	
	free(P->states);
	free(P);
}

/*****************************************************************************
name		: free_block
role       	: This method frees the block.
@param     	: block *B: The block.
******************************************************************************/
void free_block(block *B){
	free_states(B->states);
	free(B);
}

/*****************************************************************************
name		: free_states
role       	: This method frees the list of states.
@param     	: state *s: The list of states.
******************************************************************************/
void free_states(state *s){
	state *cur, *next;
	
	cur = s;
	while(cur != NULL){
		next = cur->next;
		free(cur);
		cur = next;
	}
}

/*****************************************************************************
name		: print_states
role       	: This method prints state indices.
@param     	: state *states: The list of states.
******************************************************************************/
static void print_states(state *states){
	state *s;
	BOOL bFirst = TRUE;
	
	printf("states: { ");
	for(s = states; s != NULL; s = s->next){
		if( ! bFirst ){
			printf(", ");
		}else{
			bFirst = FALSE;
		}
		printf("%d", s->index+1);
	}
	printf(" }\n");
}

/*****************************************************************************
name		: print_partition
role       	: This method prints the blocks and states of the partition.
@param     	: partition *P: The partition.
******************************************************************************/
void print_partition(partition *P){
	block *B;
	
	printf("Partition:\n");
	
	for(B = P->blocks; B != NULL; B = B->next){
		if(B->states != NULL){
			printf("state %d: ", B->row+1);
			printf("%d ", B->num_states);
			print_states(B->states);
		}
	}
}
