/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: partition.h,v $
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
*	Authors: Tim Kemna
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

#ifndef PARTITION_H
#define PARTITION_H

#include "bitset.h"

// flags for block
#define SPLITTER	0x01
#define PARTITIONED	0x02
#define ABSORBING	0x04

typedef struct reward_tree_elem Reward_Tree;
typedef struct state_elem state;
typedef struct block_elem block;

#include "splay.h"

/**
*	STRUCTURE
* A reward tree lements, is used for creating the initial partitioning.
* @member reward	: the reward value in this node
* @member subblock	: the corresponding block, note if there is a node then
*			  subblock != NULL
* @member left	: the left subtree i.e. with rewards << reward
* @member right	:  the left subtree i.e. with rewards >> reward
*/
struct reward_tree_elem {
	double reward;
	block *subblock;
	Reward_Tree *left;
	Reward_Tree *right;
};

/**
* This method is used for creating a new Reward_Tree node with the given reward rate.
* @param reward	: The reward rate
* @return the newly created node
*/
inline Reward_Tree * createNewRewardNode(const double reward);

/**
* This is a recursive procedure that is used for searching an appropriate reward node,
* and if there is no such node yet, it creates it.
* @param reward_root	: The reward tree we have to find/insert a node for the given reward
* @param reward		: The reward we are looking for
* @return the found of newly created Reward_Tree node for the given reward 
*/
Reward_Tree * getStateRewardNode(Reward_Tree * reward_root, const double reward);

/**
* This method is used to recursively free the reward tree
* NOTE: The blocks placed in the tree node are left allocated,
*	  because they are expected to be referenced from somewhere
*	  else.
* @param retard_tree	: the reward tree to be deleted
*/
void free_reward_tree(Reward_Tree * reward_tree);

/**
*	STRUCTURE
* A state in a block.
* @member index	: row index of this state.
* @member b		: pointer to the block of this state.
* @member prev	: pointer to previous state.
* @member next	: pointer to next state.
*/
struct state_elem {
	int index;
	block *b;
	state *prev;
	state *next;
};

/**
*	STRUCTURE
* A block in a partition.
* @member states		: linked-list of states in this block.
* @member tree		: pointer to the sub-block tree.
* @member num_states	: number of states in this block.
* @member row		: row index in new (lumped) matrix.
* @member flags		: set of flags.
* @member next		: pointer to next block.
*/
struct block_elem {
	state *states;
	Tree *tree;
	int num_states;
	int row;
	int flags;
	block *next;
};

/**
*	STRUCTURE
* A pointer to a block in the list of potential splitters.
* @member B		: pointer to a block.
* @member next	: pointer to next block pointer.
*/
typedef struct block_ptr block_pointer;
struct block_ptr {
	block *B;
	block_pointer *next;
};

/**
*	STRUCTURE
* @member blocks		: linked-list of blocks in the partition.
* @member last_block	: pointer to last block.
* @member states		: array of pointers to state structures.
* @member num_blocks	: number of blocks in the partition.
*/
typedef struct{
	block *blocks;
	block *last_block;
	state **states;
	int num_blocks;
} partition;

/**
* This macro returns the block of a state.
* @param	: partition *P: The partition.
* @param	: int i: The state index.
*/
#define block_of(P, i) P->states[i]->b

/**
* This method adds a state to a block.
* @param	: block *b: The block.
* @param	: state *s: The state to add.
*/
void add_state(block *b, state *s);

/**
* This method deletes a state from a block.
* @param	: partition *P: The current partition.
* @param	: block *b: The block.
* @param	: int index: The index of the state to add.
*/
state *delete_state(partition *P, block *b, int index);

/**
* This method creates and initialises an empty block.
* @return	: The block.
*/
block *init_block();

/**
* This method prints the position of the bits in the original
* bitset whose value is 1.
* @param P: The partition.
* @param b: The bitset corresponding to the lumped state space.
*/
void print_original_states(partition *P, bitset *b);

/**
* This method prints TRUE if index is in the unlumped bitset 'b',
* otherwise false.
* @param P: The partition.
* @param b: The bitset corresponding to the lumped state space.
* @param const: The original-state-space state index
*/
void print_original_state( const partition *P, const bitset *b, const int index );

/**
* Print the unlumped vector of probabilities.
* @param P: The partition.
* @param length: The length of the 'vec' array.
* @param vec: The array of probabilities to be printed.
*/
void print_state_probs_partition(partition *P , int length, double * vec);

/**
* This method prints the probability of one state only, with
* the unlumping done first.
* @param P: The partition.
* @param index: The state index in the original state space.
* @param length: The length of the 'vec' array.
* @param vec: The array of probabilities to be printed.
*/
void print_state_prob_partition( const partition *P , const int index, const int length, const double * vec);

/**
* This method frees the partition.
* @param	: partition *P: The partition.
*/
void free_partition(partition *P);

/**
* This method frees the block.
* @param	: block *B: The block.
*/
void free_block(block *B);

/**
* This method frees the list of states.
* @param	: state *s: The list of states.
*/
void free_states(state *s);

/**
* This method prints the blocks and states of the partition.
* @param	: partition *P: The partition.
*/
void print_partition(partition *P);

/**
* Here we get the original state-space size,
* that has changed because of lumping.
* @return the original state-space size
*/
inline int get_unlumped_state_space_size(partition * P);

#endif
