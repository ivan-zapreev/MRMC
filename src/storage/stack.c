/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: stack.c,v $
*	$Revision: 1.5 $
*	$Date: 2007/10/10 12:46:37 $
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
*	Source description: Manage a Stack - For Checking Steady State properties.
*	Uses: Definition of stack - stack.h
*/

#include "stack.h"

/**
* This function creates a new array of stacks
* @param n the number of required stacks
* @return the pointer to the new stackarray
*/
stack * getNewStackArray(int n){
	int i;
	//Allocate the stackarray
	stack * stackarray = (stack *)calloc(1,sizeof(stack));
	
	IF_SAFETY( stackarray != NULL )
		//Init the other stackarray fields
		stackarray->stack_num = 0;
		
		extendStackArray(stackarray, n);	
	ELSE_SAFETY
		printf("ERROR: no further memory available!");
		exit(EXIT_FAILURE);
	ENDIF_SAFETY
	
	return stackarray;
}

/**
* This function creates a new stack within an existing stackarray
* @param stackarray the array, which should be extended by new stacks
* @param n the number of stacks to extend the array with
*/
void extendStackArray(stack *stackarray, int n)
{
	int i;
	const int new_stack_num = (stackarray->stack_num) + n;
	
	//Reallocate the array of stacks in the stackarray structure
	if( n > 0){
		stackarray->stackp = (int **)realloc(stackarray->stackp, new_stack_num * sizeof(int*));
	}

	IF_SAFETY( stackarray->stackp != NULL )
		//Init the stack fields
		for(i=stackarray->stack_num; i< new_stack_num; i++) {
			stackarray->stackp[i] = (int *) calloc(1, ALLOC_BLOCK_SIZE *sizeof(int));
			
			IF_SAFETY( stackarray->stackp[i] != NULL )
				stackarray->stackp[i][1] = FIRST_ELEMENT_IDX - 1;
				stackarray->stackp[i][0] = ALLOC_BLOCK_SIZE;
			ELSE_SAFETY
				printf("ERROR: no further memory available!");
				exit(EXIT_FAILURE);
			ENDIF_SAFETY
		}
		
		//Init the other stackarray fields
		stackarray->stack_num = new_stack_num;
	ELSE_SAFETY
		printf("ERROR: no further memory available!");
		exit(EXIT_FAILURE);
	ENDIF_SAFETY
}
	
/** This function resets the top element index of the stack
* @param cur_stack a pointer to the stack which top element index should get a reset
*/
void cleanStack(int *cur_stack)
{
	cur_stack[1] = FIRST_ELEMENT_IDX - 1;
}

/**
* This function deallocates the stack which is used to store states while searching for BSCCs
* @param cur_stack the pointer to the stack to be deallocated
*/
void freeStackArray(stack *stackarray)
{
	int i;
	if( stackarray ) {
		if( stackarray->stackp ) {
			for(i=0; i < stackarray->stack_num; i++) {
				free(stackarray->stackp[i]);
			}
			free(stackarray->stackp);
		}
		free(stackarray);
	}
}

/**
* Stores an int value on the stack
* @param *cur_stack the pointer to the stack the value should be stored on
* @param n the int value to be stored
* @return the pointer to the stack, the value is stored on
*/
int * pushStack(int *cur_stack, int n)
{
	/*If the stack size is equal to the number of stored elements then we
	need to allocat new memory for one more element.*/
	if( cur_stack[0] == (cur_stack[1]) + 1) {
		cur_stack[0] += ALLOC_BLOCK_SIZE;
		
		/* Realloc frees old memory and allocates new one, if adress of new block
		not equal with the old one, the old block is copied into the new */
		cur_stack = (int *) realloc(cur_stack, (cur_stack[0]) *sizeof(int));
		
		IF_SAFETY( cur_stack != NULL )
			//Just go further with the execution
			;
		ELSE_SAFETY
			printf("ERROR: no further memory available!");
			exit(EXIT_FAILURE);
		ENDIF_SAFETY
	}
	/*Store the element*/
	cur_stack[1]++;
	cur_stack[cur_stack[1]] = n;
	
	return cur_stack;
}

/**
* This function stores a tuple of int values on the stack
* @param *cur_stack a pointer to the stack, on which the value should be stored
* @param n the first value to be stored
* @param succ the second value to be stored
* @return the pointer to the stack, the value is stored on
*/
int * pushStackTuple(int *cur_stack, int n, int succ)
{
	/* Push n on stack */
	cur_stack = pushStack(cur_stack, n);
	/*Push number of non-visited successors on stack*/
	cur_stack = pushStack(cur_stack, succ);
	
	return cur_stack;
}

/**
* Retrieves the stack elements
* @param *cur_stack the stack from which the element should be retrieved
* @return -1 if the stack is empty, otherwise the element from the top of the stack
* NOTE: cur_stack[0] and cur_stack[1] store allocated stack size and top stack idx
*/
int popStack(int *cur_stack)
{
	if( cur_stack[1] >= FIRST_ELEMENT_IDX ) {
		return cur_stack[cur_stack[1]--]; /*Decrease the index of the top 
		element "remove element"*/
	} else {
		return EMPTY_STACK;
	}
}

/**
* Returns the upper element of the stack withoput removing it from the stack.
* @param *cur_stack the pointer to the stack from which the element should be read
* @return -1 if the stack is empty, otherwise the element from the top of the stack
* NOTE: cur_stack[0] and cur_stack[1] store allocated stack size and top stack idx
*/
int getTopStack(int *cur_stack)
{
	if( cur_stack[1] >= FIRST_ELEMENT_IDX ) {
		return cur_stack[cur_stack[1]]; 
		/*Do not decrease the top element's index "leave it"*/
	} else {
		return EMPTY_STACK;
	}
}
