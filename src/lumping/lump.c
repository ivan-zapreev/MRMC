/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: lump.c,v $
*	$Revision: 1.9 $
*	$Date: 2007/10/10 13:48:31 $
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
*	Source description: Lumping of Markov chains.
*	Uses: DEF: lump.h, partition.h, splay.h, sparse.h, bitset.h, label.h, runtime.h, macro.h, 
*		LIB: partition.c, splay.c, sparse.c, bitset.c, label.c, 
*/

#include <stdio.h>
#include <stdlib.h>
#include "macro.h"
#include "bitset.h"
#include "sparse.h"
#include "label.h"
#include "partition.h"
#include "lump.h"
#include "splay.h"
#include "runtime.h"

//The number of possible blocks in the initial partitioning
//For the bounded until formula
const int INITIAL_PART_BOUNDED_U_BLOCKS = 3;
//The number of possible blocks in the initial partitioning
//For the interval until formula
const int INITIAL_PART_INTERVAL_U_BLOCKS = 4;

//State indeces for the initial partitioning of the formula dependent lumping
//The index of the failure !Phi&&!Psi state block
const int INDEX_NOT_PHI_AND_NOT_PSI_STATE = 0;
//The index of the !Phi&&Psi state block
const int INDEX_NOT_PHI_AND_PSI_STATE = 1;
//The index of the inconclusive Phi&&!Psi state block
const int INDEX_PHI_AND_NOT_PSI_STATE = 2;
//The index of the Phi&&Psi state block
const int INDEX_PHI_AND_PSI_STATE = 3;

static block_pointer *last;	//points to last element of L
static int *L1;			// L'
static double *sum;		// sum[i] == xi.sum

/**
* Predeclarations:
*/
static void split(block *S, partition *P, block_pointer *L, sparse *Q);
static void quicksort(int *cols, double* vals, int left, int right);

/**
* Descend in the binary search tree to determine the correct block for the given state
* based on the state labelling. If the corresponding node is not found then it is
* created and inserted into the tree.
* @param root		: the binary tree root
* @param labellin		: the labelling 
* @param state_index	: the state index
*/
static inline Tree * getLabelToBinaryTreeNode(const Tree * root, const labelling * labellin, const int state_index){
	int i;
	Tree * leaf = (Tree *) root;
	
	for(i = 0; i < labellin->n; i++){
		if( get_bit_val( labellin->b[i], state_index ) ){
			if(leaf->left == NULL){
				leaf->left = (Tree *) calloc(1, sizeof(Tree));
				IF_SAFETY( leaf->left != NULL )
					leaf->left->left = leaf->left->right = NULL;
					leaf->left->subblock = NULL;
					leaf->left->reward_tree = NULL;
				ELSE_SAFETY
					printf("ERROR: Fatal failure. Unable to allocate memory, lump.c::getLabelToBinaryTreeNode(...) ");
					exit(1);
				ENDIF_SAFETY
			}
			leaf = leaf->left;
		}else{
			if(leaf->right == NULL){
				leaf->right = (Tree *) calloc(1, sizeof(Tree));
				IF_SAFETY( leaf->right != NULL )
					leaf->right->left = leaf->right->right = NULL;
					leaf->right->subblock = NULL;
					leaf->right->reward_tree = NULL;
				ELSE_SAFETY
					printf("ERROR: Fatal failure. Unable to allocate memory, lump.c::getLabelToBinaryTreeNode(...) ");
					exit(1);
				ENDIF_SAFETY
			}
			leaf = leaf->right;
		}
	}
	return leaf;
}

/**
* Add an extension of the binary tree for the rewards.
* So that we would have also blocks of states with the
* same state rewards r2 << r1 << r3:
*	       r1
*	     /    \
*	    r2    r3
*	   /  \ /   \
* @param leaf		: the leaf of the binary tree that corresponds to
				  the labelling of the considered state
* @param state_index	: the considered state index
* @return the reward tree node that corresponds to the state reward
*/
static inline Reward_Tree * getRewardToBinaryTreeNode(Tree * leaf, const int state_index){
	const double * pStateRewards = getStateRewards();
	const BOOL isStateRewards = pStateRewards != NULL;
	Reward_Tree *reward_leaf;

	const double reward = isStateRewards ? pStateRewards[state_index] : 0.0;
	
	if(leaf->reward_tree == NULL){
		//In case when there is no rewards there will be just one
		//element for each reward tree with reward == 0
		reward_leaf = leaf->reward_tree = createNewRewardNode(reward);
	}else{
		//Search for the appropriate reward node, and
		//if there is no such node yet, create it
		reward_leaf = getStateRewardNode(leaf->reward_tree, reward);
	}
	
	return reward_leaf;
}

/**
* Allocates the reward_leaf->subblock if necessary and then also adds it into
* the partitioning.
* @param reward_leaf	: the reward leaf where the block should be allocated
* @param P			: the partitioning
* @return TRUE if the new block had to be allocated
*/
static inline BOOL allocateBlockOnDemand(Reward_Tree * reward_leaf, partition *P){
	BOOL result = FALSE;
	if(reward_leaf->subblock == NULL){
		reward_leaf->subblock = init_block();
		
		if(P->blocks == NULL){
			P->blocks = reward_leaf->subblock;
		}else{
			P->last_block->next = reward_leaf->subblock;
		}
		P->last_block = reward_leaf->subblock;
		result = TRUE;
	}
	
	return result;
}

/**
* This method returns an initial partition based on the labelling structure.
* The initial partition contains one block for each combination of labels.
* @param	: labelling *labellin: The labelling structure.
* @return	: The initial partition.
*/
partition *init_partition(labelling *labellin){
	partition *P;
	int state_index, i;
	state *s;
	Tree *root = (Tree *) calloc(1, sizeof(Tree));
	root->left = root->right = NULL;
	root->reward_tree = NULL;
	Tree *leaf;
	Reward_Tree *reward_leaf;
	
	P = (partition *) calloc(1, sizeof(partition));
	if(P != NULL){
		
		P->states = (state **) calloc(labellin->ns, sizeof(state *));
		P->blocks = NULL;
		P->last_block = NULL;
		
		if(P->states != NULL){
			// insert each state in a block
			for(state_index = 0; state_index < labellin->ns; state_index++){
				s = (state *) calloc(1, sizeof(state));
				if(s == NULL){
					return NULL;
				}
				s->index = state_index;
				P->states[state_index] = s;
				
				// descend in binary search tree to determine the correct block
				leaf = getLabelToBinaryTreeNode(root, labellin, state_index);
				
				// find or create a corresponding entry in the reward subtree
				reward_leaf = getRewardToBinaryTreeNode(leaf, state_index);
				
				// create block if it does not yet exists
				allocateBlockOnDemand(reward_leaf, P);
				
				// add state to the block
				add_state(reward_leaf->subblock, s);
			}
		}else{
			return NULL;
		}
	}else{
		return NULL;
	}
	
	P->last_block->next = NULL;
	
	free_tree(root); //Also cleans the Reward_Tree structure!
	
	return P;
}

/**
* Without the rewards the array index would correspond to the
* initial partitioning block for the formula dependent lumping.
* But as long as there are rewards we also have to split them,
* the latter is done by using the binary trees which sord out
* states with different state rewards. This method though just
* gives the index of the corresponding block (Reward_Tree root node)
*
*/
static inline int getTheRewardTreeArrayIndex(const bitset *phi, const bitset* psi, const BOOL interval, const int state_index)
{
	int index;
	
	if( get_bit_val( psi, state_index ) ){
		if(interval){
			if( get_bit_val( phi, state_index ) ){
				// phi && psi state
				index = INDEX_PHI_AND_PSI_STATE;
			}else{
				// !phi && psi state
				index = INDEX_NOT_PHI_AND_PSI_STATE;
			}
		}else{
			index = INDEX_NOT_PHI_AND_PSI_STATE;
		}
	}else{
		if( get_bit_val( phi, state_index ) ){
			// inconclusive (phi && !psi) state
			index = INDEX_PHI_AND_NOT_PSI_STATE;
		}else{
			// failure (!phi && !psi) state
			index = INDEX_NOT_PHI_AND_NOT_PSI_STATE;
		}
	}
	
	return index;
}

/**
* Add an extension of the binary tree for the rewards.
* So that we would have also blocks of states with the
* same state rewards r2 << r1 << r3:
*	       r1
*	     /    \
*	    r2    r3
*	   /  \ /   \
* @param reward_trees	: the array of initial partition blocks,
*				  each of which is a binary tree for the
*				  reward case. For the non-reward case we
*				  just have an array of reward nodes - single
*				  noded trees. NOTE: that initially nodes are
*				  not allocated.
* @param index		: the index of the partition elemen, i.e.
*				  an index in the reward_trees array
* @param state_index	: the considered state index
* @return the reward tree node that corresponds to the state reward
*/
static inline Reward_Tree * lookUpTheRewardNode(Reward_Tree **reward_trees, const int index, const int state_index){
	const double * pStateRewards = getStateRewards();
	const BOOL isStateRewards = pStateRewards != NULL;
	Reward_Tree *reward_leaf;

	const double reward = isStateRewards ? pStateRewards[state_index] : 0.0;
	
	if(reward_trees[index] == NULL){
		//In case when there is no reward tree here yet we
		//allocate a new one. In non reward case it will
		//be just one node tree with thre reward rate == 0.0
		reward_leaf = reward_trees[index] = createNewRewardNode(reward);
	}else{
		//Search for the appropriate reward node, and
		//if there is no such node yet, create it
		reward_leaf = getStateRewardNode(reward_trees[index], reward);
	}
	
	return reward_leaf;
}

/**
* This method returns a formula-dependent initial partition.
* @param	: bitset *phi: satisfaction relation for phi formula.
* @param	: bitset *psi: satisfaction relation for psi formula.
* @param	: BOOL interval: CSL interval until formula, if true
*		  PCTL/CSL (un)bounded until formula, otherwise
* @return The initial partition:
*	   {Sat(psi), Sat(phi && !psi), Sat(!psi && !psi)} for (un)bounded until formulas.
*	   {Sat(!phi && psi), Sat(phi && !psi), Sat(!phi && !psi), Sat(phi && psi)}
*	   for interval until formulas.
*/
partition *init_partition_formula(const bitset *phi, const bitset* psi, const BOOL interval){
	partition *P;
	int state_index, i, index;
	//Reward tree is here just for the sake of rewards model checking
	//Otherwise we will get just an array of single node trees like
	//we would have an array of blocks without rewards
	Reward_Tree **reward_trees, * reward_leaf;
	state *s;
	int num_blocks = INITIAL_PART_BOUNDED_U_BLOCKS;
	
	if(interval){
		num_blocks = INITIAL_PART_INTERVAL_U_BLOCKS;
	}
	
	//Allocate an array of Reward_Tree*, i.e. create blocks for
	//the initial partition, NOTE: Right now there are no tree nodes allocated!
	reward_trees = (Reward_Tree **) calloc(num_blocks, sizeof(Reward_Tree *));
	
	P = (partition *) calloc(1, sizeof(partition));
	
	if(P != NULL && reward_trees != NULL){
		
		P->states = (state **) calloc(phi->n, sizeof(state *));
		P->blocks = NULL;
		P->last_block = NULL;
		
		if(P->states != NULL){
			// insert each state in a block
			for(state_index = 0; state_index < phi->n; state_index++){
				s = (state *) calloc(1, sizeof(state));
				if(s == NULL){
					return NULL;
				}
				s->index = state_index;
				P->states[state_index] = s;
				
				// determine index in array of reward trees
				index = getTheRewardTreeArrayIndex(phi, psi, interval, state_index);
				
				// search and if necessary allocate the proper Reward_Tree node
				reward_leaf = lookUpTheRewardNode(reward_trees, index, state_index);
				
				// create block if it does not yet exists
				if ( allocateBlockOnDemand(reward_leaf, P) ){
					// make pure failure and pure success states absorbing
					if(index == INDEX_NOT_PHI_AND_NOT_PSI_STATE || index == INDEX_NOT_PHI_AND_PSI_STATE){
						// make blocks absorbing
						set_flag(reward_leaf->subblock->flags, ABSORBING);
					}
				}
				
				// add state to the block
				add_state(reward_leaf->subblock, s);
			}
		}else{
			return NULL;
		}
	}else{
		return NULL;
	}
	
	P->last_block->next = NULL;
	
	//Free the array of Reward_Tree structures
	for(i = 0; i < num_blocks; i++){
		free_reward_tree(reward_trees[i]);
	}
	free(reward_trees);
	
	return P;
}

/*****************************************************************************
name		: init_splitters
role       	: This method returns an linked-list of pointers to blocks, one for each block in P.
@param     	: partition *P: The initial partition.
@return     : The linked-list of pointers to blocks.
******************************************************************************/
block_pointer *init_splitters(partition *P){
	block *blocks;
	block_pointer *temp, *L = NULL;
	
	blocks = P->blocks;
	L = (block_pointer *) calloc(1, sizeof(block_pointer));
	if(L == NULL){
		return NULL;
	}
	L->B = blocks;
	temp = L;
	set_flag(blocks->flags, SPLITTER);
	for(blocks = blocks->next; blocks != NULL; blocks = blocks->next){
		set_flag(blocks->flags, SPLITTER);
		temp->next = (block_pointer *) calloc(1, sizeof(block_pointer));
		if(temp->next == NULL){
			return NULL;
		}
		temp = temp->next;
		temp->B = blocks;
	}
	
	temp->next = NULL;
	last = temp;

	return L;
}

/*****************************************************************************
name		: lump
role       	: This method computes the optimal lumped matrix of the input matrix.
@param     	: partition *P: The initial partition.
@param     	: sparse *Q: The matrix to lump.
@return     : The lumped matrix.
******************************************************************************/
sparse *lump(partition *P, sparse *Q){
	block *B, *prev, *next;
	block_pointer *S, *L;
	int n, xi, col, row, count;
	sparse *Q1;	// Q'
	double val;
	
	// line 1
	L = init_splitters(P);
	if(L == NULL){
		printf("ERROR: Allocating storage for splitters failed\n");
		exit(1);
	}

	//line 1 of SPLIT	
	L1 = (int *) calloc(1, Q->rows * sizeof(int));
	if(L1 == NULL){
		printf("ERROR: Allocating storage for L1 failed\n");
		exit(1);
	}
	sum = (double *) calloc(Q->rows, sizeof(double));
	if(sum == NULL){
		printf("ERROR: Allocating storage for sum failed\n");
		exit(1);
	}
	
	// lines 2-4
	while(L != NULL){
		S = L;
		clear_flag(S->B->flags, SPLITTER);
		split(S->B, P, L, Q);
		L = L->next;
		free(S);
	}
	
	free(sum);
	sum = NULL;
	
	// line 5 + determines row number for blocks + free empty blocks
	n = 0;
	prev = NULL;
	B = P->blocks;
	while(B != NULL){
		next = B->next;
		if(B->num_states > 0){
			B->row = n++;
			prev = B;
		}else{
			free(B);
			if(prev != NULL){
				prev->next = next;
			}else{
				P->blocks = next;
			}
		}
		B = next;
	}
	P->num_blocks = n;

	// determine number of unique outgoing transitions for each block
	int *ncolse = (int *)calloc(n, sizeof(int));
	if(ncolse == NULL){
		printf("ERROR: Allocating storage for matrix failed\n");
		exit(1);
	}
	// L1 is now used to store if a block is a successor
	memset(L1, -1, n * sizeof(int));	// init L1[i] with -1 for all i<n
	for(B = P->blocks; B != NULL; B = B->next){
		if(test_flag(B->flags, ABSORBING)){
			continue;
		}
		xi = B->states->index;
		count = 0;
		row = B->row;
		while(get_mtx_next_val(Q, xi, &col, &val, &count)){
			if(L1[block_of(P,col)->row] != row){
				L1[block_of(P,col)->row] = row;
				++ncolse[row];
			}
		}
	}
	free(L1);
	L1 = NULL;
	
	// line 6-7
	Q1 = init_matrix_ncolse(n, n, ncolse);
	if(Q1 == NULL){
		printf("ERROR: Allocating storage for matrix failed\n");
		exit(1);
	}
	free(ncolse);
	ncolse = NULL;
	
	// line 8-12
	for(B = P->blocks; B != NULL; B = B->next){
		row = B->row;
		if(test_flag(B->flags, ABSORBING)){
			// make this block absorbing (only needed for formula-dependent lumping)
			if(isRunMode(PCTL_MODE) || isRunMode(PRCTL_MODE)){
				set_mtx_val_ncolse(Q1, row, row, 1.0);
			}
			continue;
		}
		// line 9
		xi = B->states->index;
		// line 10-12
		count = 0;
		while(get_mtx_next_val(Q, xi, &col, &val, &count)){
			add_mtx_val_ncolse(Q1, row, block_of(P,col)->row, val);
		}
		if(get_mtx_diag_val(Q, xi, &val)){
			add_mtx_val_ncolse(Q1, row, row, val);
		}
		// order row by column index
		if(Q1->ncols[row] > 1){
			quicksort(Q1->val[row].col, Q1->val[row].val, 0, Q1->ncols[row]-1);
		}
	}
	printf("Lumping: The number of partition blocks is %d\n", Q1->rows);
	
	// line 13
	return Q1;
}

/*****************************************************************************
name		: split
role       	: This method refines the current partition with respect to the splitter, 
			  and may also add a number of new potential splitters to L.
@param     	: block *S: The splitter.
@param     	: partition *P: The current partition.
@param     	: block_pointer *L: The list of potential splitters.
@param     	: sparse *Q: The matrix to lump.
******************************************************************************/
static void split(block *S, partition *P, block_pointer *L, sparse *Q){
	block_pointer *L2 = NULL;	// L''
	block_pointer *b_ptr;
	int xj, xi, i, max, next = 0;
	double val;
	block *B, *subblocks, *largest, *sub;
	state *s;
	int *back_set;

	if(S->num_states == 0){
		return;
	}

	// line 5-8: compute sum values
	for(s = S->states; s != NULL; s = s->next){
		xj = s->index;
		max = Q->pcols[xj];
		back_set = Q->val[xj].back_set;	// array with predecessors of xj
		for(i = 0; i < max; i++){
			xi = back_set[i];
			// do not split this block, because it will be absorbing
			//(only needed for formula-dependent lumping)
			if(test_flag(block_of(P,xi)->flags, ABSORBING)) continue;
			BOOL wasFound = get_mtx_val(Q, xi, xj, &val);
			IF_SAFETY( wasFound )
				if(sum[xi] == 0.0){
					L1[next++] = xi;	// line 8
				}
				sum[xi] += val;			// line 7
			ELSE_SAFETY
				printf("ERROR: The .tra file is not ordered correctly.\n");
				exit(1);
			ENDIF_SAFETY
		}
		if(test_flag(S->flags, ABSORBING)) continue; // do not split the splitter, because it will be absorbing (only needed for formula-dependent lumping)
		if(get_mtx_diag_val(Q, xj, &val)){
			if(sum[xj] == 0.0){
				L1[next++] = xj;	// line 8
			}
			sum[xj] += val;			// line 7
		}
	}
	
	// line 9-13: remove predecessors from original block and insert into sub-block tree
	for(next--; next >= 0; next--){
		xi = L1[next];
		// line 10
		B = block_of(P,xi);
		// line 11
		s = delete_state(P, B, xi);
		//line 12
		B->tree = insert(sum[xi], B->tree);
		add_state(B->tree->subblock, s);
		
		sum[xi] = 0.0;	// initialise to zero for next call to split()
		
		//line 13
		if(!test_flag(B->flags, PARTITIONED)){	// if not in L2, add to the list L2
			b_ptr = (block_pointer *) calloc(1, sizeof(block_pointer));
			IF_SAFETY( b_ptr != NULL )
				b_ptr->B = B;
				if(L2 == NULL){
					b_ptr->next = NULL;
				}else{
					b_ptr->next = L2;
				}
				L2 = b_ptr;
				set_flag(B->flags, PARTITIONED);	// B is in L2
			ELSE_SAFETY
				printf("ERROR: Allocating storage for L'' failed\n");
				exit(1);
			ENDIF_SAFETY
		}
	}

	// line 14-20: add sub-blocks to the partition and add potential splitters
	while(L2 != NULL){
		B = L2->B;
		
		b_ptr = L2->next;
		free(L2);
		L2 = b_ptr;

		sub = P->last_block;
		// destroy sub-block tree and append sub-blocks to partition
		P->last_block = destroy_tree(B->tree, P->last_block);
		subblocks = sub->next;	// pointer to the first sub-block
		
		B->tree = NULL;
		clear_flag(B->flags, PARTITIONED);	// B is not in L''
		
		if(subblocks->next == NULL && B->num_states == 0){
			// original block contains no states anymore and it has only 1 sub-block
			if(test_flag(B->flags, SPLITTER)){
				// original block is a potential splitter, so the sub-block should also be a potential splitter
				// the original empty block is not deleted
				set_flag(subblocks->flags, SPLITTER);
				clear_flag(B->flags, SPLITTER);
				last->next = (block_pointer *) calloc(1, sizeof(block_pointer));
				IF_SAFETY( last->next != NULL )
					last = last->next;
					last->B = subblocks;
					last->next = NULL;
				ELSE_SAFETY
					printf("ERROR: Allocating storage for L failed\n");
					exit(1);
				ENDIF_SAFETY
			}
			/*
			// delete empty original block and move states from the sub-block to the original block
			// this takes more time than leaving an empty block, but it requires less memory
			B->states = subblocks->states;
			for(s = B->states; s != NULL; s = s->next){
				s->b = B;
			}
			B->num_states = subblocks->num_states;
			P->last_block = sub;
			P->last_block->next = NULL;
			free(subblocks);
			*/
		}else{
			largest = NULL;	// pointer to the largest sub-block
			if(test_flag(B->flags, SPLITTER)){
				// original block is a potential splitter, so we cannot neglect the largest sub-block
				largest = NULL;
			}else{
				// determine the largest sub-block
				max = 0;
				for(sub = subblocks; sub != NULL; sub = sub->next){
					if(sub->num_states > max){	// check if this subblock is larger
						max = sub->num_states;
						largest = sub;
					}
				}
				
				if(B->num_states > 0 && B->num_states <= max){
					// the original block is not larger than the largest sub-block
					last->next = (block_pointer *) calloc(1, sizeof(block_pointer));
					IF_SAFETY( last->next != NULL)
						last = last->next;
						last->B = B;
						last->next = NULL;
						//original block is a potential splitter
						set_flag(B->flags, SPLITTER);
					ELSE_SAFETY
						printf("ERROR: Allocating storage for L failed\n");
						exit(1);
					ENDIF_SAFETY
				}else{
					// the original block is larger than the largest sub-block
					largest = B;	// neglect original block as potential splitter
				}
			}
			if(B->num_states == 0){
				clear_flag(B->flags, SPLITTER);
			}
			
			// add sub-blocks to L, except the largest one
			for(sub = subblocks; sub != NULL; sub = sub->next){
				if(sub != largest){
					last->next = (block_pointer *) calloc(1, sizeof(block_pointer));
					IF_SAFETY( last->next != NULL)
						last = last->next;
						last->B = sub;
						last->next = NULL;
						// subblock is a potential splitter
						set_flag(sub->flags, SPLITTER);
					ELSE_SAFETY
						printf("ERROR: Allocating storage for L failed\n");
						exit(1);
					ENDIF_SAFETY
				}else{
					clear_flag(sub->flags, SPLITTER);	// subblock is not a potential splitter
				}
			}
		}
	}
	last->next = NULL;
}

/*****************************************************************************
name		: quicksort
role       	: This method sorts the row elements by column index.
@param     	: int *cols: column indices of non-zero elements.
@param     	: int *vals: values of non-zero elements.
@param     	: int left: first index.
@param     	: int right: last index.
******************************************************************************/
static void quicksort(int *cols, double* vals, int left, int right)
{
	int i, j, x, y;
	double z;
 
	i = left; j = right;
	x = cols[(left+right)/2];
 
	do {
		while((cols[i] < x) && (i < right)) i++;
		while((x < cols[j]) && (j > left)) j--;

		if(i <= j) {
			y = cols[i];
			cols[i] = cols[j];
			cols[j] = y;
			
			z = vals[i];
			vals[i] = vals[j];
			vals[j] = z;
			
			i++; j--;
		}
	} while(i <= j);

	if(left < j) quicksort(cols, vals, left, j);
	if(i < right) quicksort(cols, vals, i, right);
}

/**
* This method changes the labelling structure such that it corresponds to the partition.
* @param     	: labelling *labellin: the labelling structure.
* @param     	: partition *P: the partition.
*/
void change_labelling(labelling *labellin, partition *P){
	block *B;
	bitset **b;
	int label, s;
	
	b = (bitset **)calloc(labellin->n,sizeof(bitset *));
	for(label = 0; label < labellin->n; label++){
		b[label] = get_new_bitset(P->num_blocks);
	}
	
	for(B = P->blocks; B != NULL; B = B->next){
		s = B->states->index;	//arbitrary state of block
		for(label = 0; label < labellin->n; label++){
			if( get_bit_val(labellin->b[label], s) ){
				set_bit_val(b[label], B->row, BIT_ON);
			}
		}
	}
	
	for(label = 0; label < labellin->n; label++){
		free_bitset(labellin->b[label]);
	}
	free(labellin->b); //Have to free the array itself also.
	
	labellin->b = b;
	labellin->ns = P->num_blocks;
}


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
double * change_state_rewards(double * p_old_rew, partition *P){
	block * pBlock = NULL;
	double * p_new_rew = NULL;
	
	//Test whether the state rewards are present.
	if ( p_old_rew ){
		p_new_rew = (double *)calloc(P->num_blocks,sizeof(double));
		
		//Take an arbitrary state of block, because all states
		//of the same block must have the same state reward
		for(pBlock = P->blocks; pBlock != NULL; pBlock = pBlock->next){
			p_new_rew[pBlock->row] = p_old_rew[pBlock->states->index];
		}
	}
	return p_new_rew;
}

/*****************************************************************************
name		: unlump_vector
role		: unlump a probability vector with respect to the partition.
@param		: partition *P: the partition of the original state space.
@param		: int size: the number of states in the original state space.
@param		: double *result_lumped: the probability vector of the lumped state space.
@return	: the probability vector of the original state space.
******************************************************************************/
double *unlump_vector(partition *P, int size, double *result_lumped){
	int i;
	double *result = (double *) calloc(size, sizeof(double));
	for(i=0;i<size;i++){
		result[i] = result_lumped[block_of(P,i)->row];
	}
	
	return result;
}

/*****************************************************************************
name		: lump_bitset
role		: lump a bitset with respect to the partition.
@param		: partition *P: the partition of the original state space.
@param		: bitset *phi: the original bitset.
@return	: the lumped bitset.
******************************************************************************/
bitset *lump_bitset(partition *P, bitset *phi){
	block *B;
	bitset *phi_lumped = get_new_bitset(P->num_blocks);
	
	for(B = P->blocks; B != NULL; B = B->next){
		// if B is labeled with phi
		if( get_bit_val( phi, B->states->index )){
			set_bit_val(phi_lumped, B->row, BIT_ON);
		}
	}
	
	return phi_lumped;
}
