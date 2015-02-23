/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: splay.c,v $
*	$Revision: 1.2 $
*	$Date: 2007/10/10 12:46:37 $
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
*	Source description: Splay-tree implementation used for lumping.
*	Uses: DEF: splay.h, 
*	
*	"Splay trees", or "self-adjusting search trees" are a simple and
*	efficient data structure for storing an ordered set.  The data
*	structure consists of a binary tree, without parent pointers, and no
*	additional fields.  It allows searching, insertion, deletion,
*	deletemin, deletemax, splitting, joining, and many other operations,
*	all with amortized logarithmic performance.  Since the trees adapt to
*	the sequence of requests, their performance on real access patterns is
*	typically even better.  Splay trees are described in a number of texts
*	and papers [1,2,3,4,5].
*	
*	The original code was written by Daniel Sleator, and is released
*	in the public domain. The code is adapted from simple top-down splay, at the bottom of
*	page 669 of [3].  It can be obtained via anonymous ftp from
*	spade.pc.cs.cmu.edu in directory /usr/sleator/public.
*	
*	[1] "Fundamentals of data structures in C", Horowitz, Sahni,
*	    and Anderson-Freed, Computer Science Press, pp 542-547.
*	[2] "Data Structures and Their Algorithms", Lewis and Denenberg,
*	    Harper Collins, 1991, pp 243-251.
*	[3] "Self-adjusting Binary Search Trees" Sleator and Tarjan,
*	    JACM Volume 32, No 3, July 1985, pp 652-686.
*	[4] "Data Structure and Algorithm Analysis", Mark Weiss,
*	    Benjamin Cummins, 1992, pp 119-130.
*	[5] "Data Structures, Algorithms, and Performance", Derick Wood,
*	    Addison-Wesley, 1993, pp 367-375.
*	
*	List of changes:
*	- double item instead of int item
*	- modified in splay() and insert(): item values are compared using the macro equals()
*	- added functions: init_tree, destroy_tree, free_tree
*/

#include <stdio.h>
#include <stdlib.h>
#include "splay.h"

/*****************************************************************************
name		: splay
role       	: This method searches for an item with key i in the tree rooted at t.
			  If it's there, it is splayed to the root. If it isn't there, 
			  then the node put at the root is the last one before NULL that
			  would have been reached in a normal binary search for i.
@param     	: double i: the key.
@param     	: Tree *t: root of the tree.
@return     : pointer to the (new) root of the tree.
remark		: Simple top down splay, not requiring i to be in the tree t.
******************************************************************************/
Tree *splay (double i, Tree *t) {
	Tree N, *l, *r, *y;
	if (t == NULL) return t;
	N.left = N.right = NULL;
	l = r = &N;

	for (;;) {
		if(equals(i,t->item)){	
			break;
		} else if (i < t->item) {
			if (t->left == NULL) break;
			if (i < t->left->item && !equals(i, t->left->item)) {
				y = t->left;                           /* rotate right */
				t->left = y->right;
				y->right = t;
				t = y;
				if (t->left == NULL) break;
			}
			r->left = t;                               /* link right */
			r = t;
			t = t->left;
		//} else if (i > t->item) {
		} else {
			if (t->right == NULL) break;
			if (i > t->right->item && !equals(i, t->right->item)) {
				y = t->right;                          /* rotate left */
				t->right = y->left;
				y->left = t;
				t = y;
				if (t->right == NULL) break;
			}
			l->right = t;                              /* link left */
			l = t;
			t = t->right;
		}
	}
	l->right = t->left;                                /* assemble */
	r->left = t->right;
	t->left = N.right;
	t->right = N.left;
	return t;
}

/*****************************************************************************
name		: insert
role       	: This method inserts i into the tree t, unless it's already there.
@param     	: double i: the key.
@param     	: Tree *t: tree node to insert.
@return     : a pointer to the resulting tree.
******************************************************************************/
Tree *insert(double i, Tree *t) {
	Tree *new;

	if (t == NULL) {
		new = init_tree(i);
		new->left = new->right = NULL;
		return new;
	}
	t = splay(i,t);
	if(equals(i,t->item)){
//	if(i == t->item){
		 /* We get here if it's already in the tree */
             /* Don't add it again                      */
		return t;
	} else if (i < t->item) {
		new = init_tree(i);
		new->left = t->left;
		new->right = t;
		t->left = NULL;
		return new;
	//} else if (i > t->item) {
	} else {
		new = init_tree(i);
		new->right = t->right;
		new->left = t;
		t->right = NULL;
		return new;
	}
}

/*****************************************************************************
name		: init_tree
role       	: This method creates and initialises a tree node.
@param     	: double i: the key.
@return     : a pointer to tree node.
******************************************************************************/
Tree *init_tree(double i){
	Tree *new = (Tree *) calloc(1, sizeof(Tree));
	
	IF_SAFETY( new != NULL )
		new->item = i;
		new->subblock = init_block();
		new->reward_tree = NULL;
	ELSE_SAFETY
		printf("ERROR: Ran out of space splay.c::init_tree(...)\n");
		exit(1);
	ENDIF_SAFETY
	
	return new;
}

/*****************************************************************************
name		: destroy_tree
role		: This method destroys the tree by freeing the tree nodes and 
		  the sub-blocks are appended to the block_list.
@param		: Tree *t: the tree.
@param		: block *block_list: block_pointer to which the sub-blocks can be appended.
@return	: a pointer to the last appended block.
******************************************************************************/
block *destroy_tree(Tree *t, block *block_list){
	block *last_block;
	
	last_block = t->subblock;
	block_list->next = last_block;
	
	if(t->left != NULL){
		last_block = destroy_tree(t->left, last_block);
	}
	if(t->right != NULL){
		last_block = destroy_tree(t->right, last_block);
	}
	last_block->next = NULL;
	
	free(t);
	
	return last_block;
}

/*****************************************************************************
name		: free_tree
role       	: This method frees the tree.
@param     	: Tree *t: the tree.
******************************************************************************/
void free_tree(Tree *t){
	if(t->left != NULL){
		free_tree(t->left);
	}
	if(t->right != NULL){
		free_tree(t->right);
	}
	
	if(t->reward_tree != NULL){
		free_reward_tree(t->reward_tree);
	}
	
	free(t);
}
