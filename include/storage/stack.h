/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: stack.h,v $
*	$Revision: 1.1 $
*	$Date: 2007/10/01 12:40:00 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2007.
*	Authors: Ivan Zapreev, Christina Jansen
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
*	Source description: Define a Stack - For Checking Steady State properties.
*	Uses: Definition of stack - stack.h
*/
#include "stdlib.h"
#include "macro.h"

#ifndef STACK_H

#define STACK_H

/* This is the block-size stacks are extended with */
#define ALLOC_BLOCK_SIZE 128
/* This is the value to be returned, if a stack is empty */
#define EMPTY_STACK -1
/* This is the array index, from where on the stack elements are stored */
#define FIRST_ELEMENT_IDX 2

/**
* The stack structure
* @member stack_num the number of elements (stacks) in the stackp array.
* @member stackp the pointer to an array of Integers.
* NOTE: stackp[i][0] contains the allocated stack size of stack i
*	stackp[i][1] contains the top element idx of stack i
*/
typedef struct stack
{
	int stack_num;
	int **stackp;
}stack;


/**
* This function creates a new stackarray
* @param n the number of required stacks
* @return *cur_stack the pointer to the new stackarray
*/
extern stack * getNewStackArray(int n);

/**
* This function creates a new stack within an existing stackarray
* @param *stackarray the array, which should be extended by new stacks
* @param n the number of stacks to extend the array with
* @return the pointer to the extended stackarray
*/
extern void extendStackArray(stack *stackarray, int n);

/** This function resets the top element index of the stack
* @param cur_stack a pointer to the stack which top element index should get a reset
*/
void cleanStack(int *cur_stack);

/**
* This function deallocates a stack which is used to store states while searching for BSCCs
* @param *cur_stack a pointer to the stack, which should be deallocated
*/
extern void freeStackArray(stack *stackarray);

/**
* This function stores an int value on the stack
* @param cur_stack a pointer to the stack, on which the value should be stored
* @param n the int value to be stored
* @return the pointer to the stack, the value is stored on
*/
extern int * pushStack(int *cur_stack, int n);

/**
* This function stores a tuple of int values on the stack
* @param *cur_stack a pointer to the stack, on which the value should be stored
* @param n the first value to be stored
* @param succ the second value to be stored
* @return the pointer to the stack, the value is stored on
*/
extern int * pushStackTuple(int *cur_stack, int n, int succ);


/**
* This function retrieves the stack elements
* @param *cur_stack a pointer to the stack, whose element should be retrieved
* @return -1 if the stack is empty, otherwise the element from the top of the stack
*/
extern int popStack(int *cur_stack);


/**
* This function returns the upper element of the stack withoput removing it from the stack.
* If the stack is empty returns -1.
* @param *cur_stack the pointer to the stack from which the element should be read
* @return -1 if the stack is empty, otherwise the element from the top of the stack
*/
extern int getTopStack(int *cur_stack);

#endif
