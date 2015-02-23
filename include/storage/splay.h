/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: splay.h,v $
*	$Revision: 1.1 $
*	$Date: 2007/10/01 12:40:00 $
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
*	- modified the tree_node structure for the partitioning with and without rewards
*	- double item instead of int item
*	- modified in splay() and insert(): item values are compared using the macro equals()
*	- added functions: init_tree, destroy_tree, free_tree
*/

#ifndef SPLAY_H
#define SPLAY_H

#include <math.h>

typedef struct tree_node Tree;

#include "partition.h"

/**
*	STRUCTURE
* A tree node in a splay tree.
* @member left		: left child tree.
* @member right		: right child tree.
* @member item		: the key of this node.
* @member subblock		: pointer to the subblock of this tree node.
*				  NOTE: Is not used for the initial partitioning!
* @member reward_tree	: a pointer to the reward tree for the leaf node.
*				  NOTE: For the case when there is no rewards it contains
*				  just one node tree. Is mainly used for the initial
*				  partitioning with rewards.
*/
struct tree_node {
	Tree *left, *right;
	double item;
	block *subblock;
	Reward_Tree *reward_tree;
};

/*****************************************************************************
name		: equals
role       	: This macro returns whether two doubles are equal within a certain precision.
******************************************************************************/
#define EPSILON 3E-12
#define equals(x,y) (x == y || fabs(x-y) < EPSILON)

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
Tree *splay (double i, Tree *t);

/*****************************************************************************
name		: insert
role       	: This method inserts i into the tree t, unless it's already there.
@param     	: double i: the key.
@param     	: Tree *t: tree node to insert.
@return     : a pointer to the resulting tree.
******************************************************************************/
Tree *insert(double i, Tree *t);

/*****************************************************************************
name		: init_tree
role       	: This method creates and initialises a tree node.
@param     	: double i: the key.
@return     : a pointer to tree node.
******************************************************************************/
Tree * init_tree(double i);

/*****************************************************************************
name		: destroy_tree
role       	: This method destroys the tree by freeing the tree nodes and 
			  the sub-blocks are appended to the block_list.
@param     	: Tree *t: the tree.
@param     	: block *block_list: block_pointer to which the sub-blocks can be appended.
@return     : a pointer to the last appended block.
******************************************************************************/
block *destroy_tree(Tree * t, block* block_list);

/*****************************************************************************
name		: free_tree
role       	: This method frees the tree.
@param     	: Tree *t: the tree.
******************************************************************************/
void free_tree(Tree *t);

#endif
