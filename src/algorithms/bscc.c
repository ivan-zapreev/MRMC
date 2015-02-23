/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: bscc.c,v $
*	$Revision: 1.13 $
*	$Date: 2007/10/16 14:43:09 $
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
*	Source description: This file contains functions for the BSCCs detection.
*	   The algorithm is based on the Tarjan's algorithm for searching SCCs.
*/

#include "bscc.h"

/*
The stack array structure that stores all the needed stacks,
The bscc_stack is one of its elements
*/
static stack *stackarray = NULL;

/*Stack needed for the bscc search */
static int *bscc_stack = NULL;

/******************THE BSCCs SEARCH FUNCTIONS*********************************/

/*This is a holder for the rate matrix of the CTMC*/
static sparse * pStateSpace = NULL;

/*This bitset stores the visited states;*/
static bitset *pVisitedStates = NULL;

/*This bitset stores states that belong to some component
or from which we can reach a component;*/
static bitset *pInComponentStates = NULL;

/*This array stores mapping from node ids to the BSCC ids
The index of the array is the index of node and value in the corresponding
Cell is the id of the BSCC to which the given node belongs
If it is 0 then the BSCC was not found yet or it doesn't belong to any BSCC*/
static int * pBSCCs = NULL;

/*The skip variable is used to exit from the recursion
in the visit(int) function when the search should be stopped.*/
static BOOL bGlobalSkip = FALSE;

/*This variable stores the DFS order*/
static int dfs_order = 1;

/*This variable stores the id of the lately found bscc*/
static int bscc_counter = 0;

/**
* @param pBSCCsHolder the structure to retrieve bscc_counter from
* @return the number of BSCCs found so far.
*/
int getBSCCCounter(TBSCCs * pBSCCsHolder)
{
	return pBSCCsHolder->bscc_counter;
}

/**
* This method returns a mapping array from states into the BSCCs' ids.
* @param pBSCCsHolder the structure to retrieve BSCCs mapping from
* @return The array which stores mapping from node ids into the BSCC ids
* The index of the array is the index of node and value in the corresponding
* Cell is the id of the BSCC to which the given node belongs
* If it is 0 then the BSCC was not found yet or it doesn't belong to any BSCC
*/
int * getStatesBSCCsMapping(TBSCCs * pBSCCsHolder)
{
	return pBSCCsHolder->pBSCCs;
}

/**
* This function simply instantiates and returns the pointer to TBSCCs struct
* @param pStateSpaceTmp a state space to work with
* @return an empty TBSCCs structure
*/
TBSCCs * allocateTBSCCs(sparse * pStateSpaceTmp)
{
	/*Allocate structure*/
	TBSCCs * pBSCCsHolder = (TBSCCs *) calloc(1, sizeof(TBSCCs));
	
	pBSCCsHolder->pStateSpace = pStateSpaceTmp;
	pBSCCsHolder->size = pStateSpaceTmp->rows;
	
	/*Create a visited states set if this is the first search*/
	pBSCCsHolder->pVisitedStates = get_new_bitset(pBSCCsHolder->size);
	pBSCCsHolder->pInComponentStates = get_new_bitset(pBSCCsHolder->size);
	pBSCCsHolder->pBSCCs = (int *) calloc(pBSCCsHolder->size, sizeof(int));
	pBSCCsHolder->dfs_order = 1;
	pBSCCsHolder->bscc_counter = 0;
	
	return pBSCCsHolder;
}

/**
* This method is used to deallocate TBSCCs and all associated data.
* pBSCCsHolder the pointer to the structure to be cleaned
*/
void freeTBSCC(TBSCCs * pBSCCsHolder)
{
	if( pBSCCsHolder ) {
		if( pBSCCsHolder->pInComponentStates ) {
			free_bitset(pBSCCsHolder->pInComponentStates);
		}
		if( pBSCCsHolder->pVisitedStates ) {
			free_bitset(pBSCCsHolder->pVisitedStates);
		}
		if( pBSCCsHolder->pBSCCs ) {
			free(pBSCCsHolder->pBSCCs);
		}
		free(pBSCCsHolder);
	}
}

/**
* This functions checks whether the given state was already visited or not.
* @param i the state id
* @return true if the state was not yet visited.
*/
static inline BOOL isNotVisited(int i)
{
	return ( get_bit_val( pVisitedStates, i ) ? FALSE : TRUE );
}

/**
* Marks the state i as visitedfreeStack()
* @param ippBSCCs the state that was visited
*/
static inline setVisited(int i)
{
	set_bit_val(pVisitedStates, i, BIT_ON);
}

/***************THE ROOT ARRAY MANAGEMENT PROCEDURES***************************/

/*This array is used to store the root values of the nodes*/
static int * pRoot = NULL;

/**
* This method initializes the root array of length size
*/
static inline void initRoot(int size)
{
	pRoot = (int *) calloc(size, sizeof(int));
}

/**
* This method is used to free the memory used by the root array
*/
static inline void freeRoot()
{
	free(pRoot);
	pRoot = NULL;
}

/**
* Retrieves the root value for the given node
* @param v the node id
* @return the root value
*/
static inline int getRoot(int v)
{
	return pRoot[v];
}

/**
* Stores the root value for the node v
* @param v the node id
* @param val the root value
*/
static inline void setRoot(int v, int val)
{
	pRoot[v] = val;
}

/****************THE BSCCS LIST MANAGEMENT*************************************/
/*The temporary storage for the lately found BSCCs' ids*/
static int ** ppNewBSCCs = NULL;

/**
* This function is used to return the number of BSCCs known in _ppBSCCs.
* This number is stored in the 0 element of the ppNewBSCCs array, so it
* is a pointer (int *) which is used to satore an integer value only.
* @param _ppBSCCs the structure containing BSCCs along
* with the number of them, which is stored in _ppBSCCs[0]
* @return the number of BSCCs newly found (_ppBSCCs[0])
**/
inline int getBSCCCount(int ** _ppBSCCs){
	int* num_tmp = (int *) ( (void *) _ppBSCCs[0] );
	return (int) ( (unsigned long int) num_tmp );
}

/**
* This method is used to increase the BSCCs counter in _ppBSCCs[0].
* It increases _ppBSCCs[0] by one.
* @param _ppBSCCs the structure containing BSCCs along
* with the number of them, which is stored in _ppBSCCs[0]
*/
static inline void increaseBSCCCount(int ** _ppBSCCs){
	int* num_tmp = (int *) ( (void *) _ppBSCCs[0] );
	int num_tmp_tmp = (int) ( (unsigned long int) num_tmp );
	_ppBSCCs[0] = (int *) ( (unsigned long int) ( num_tmp_tmp + 1) );
}


/**
* This metod initializes the ppBSCCs array. It allocates the one
* value and stores the zero value there as it is the initial number
* of BSCCs.
*/
static void initBSCCsList()
{
	ppNewBSCCs = (int ** ) calloc( 1, sizeof( int * ) );
	/*Store the initial amount of BSCCs (it is zero)*/
	ppNewBSCCs[0] = 0;
}

/**
* This method is used to free memory used for storing BSCCs
* @param ppLastBSCCs the set of BSCCs returned by the
*                   int ** getNewBSCCs(bitset *)
*                   function.
*/
void freeBSCCs(int ** ppLastBSCCs)
{
	if(ppLastBSCCs) {
		int i, num = getBSCCCount(ppLastBSCCs);
		for( i = 1; i <= num ; i++ ) {
			free(ppLastBSCCs[i]);
		}
		free(ppLastBSCCs);
	}
}

/**
* Returns the found BSCCs list
* @return the list of BSCCS
*/
static inline int ** getNewBSCCsList()
{
	return ppNewBSCCs;
}

/**
* This method adds a new BSCC component.
* Increases the bscc_counter value by 1
*/
static void addBSCCToTheList()
{
	increaseBSCCCount(ppNewBSCCs);
	int lid = getBSCCCount(ppNewBSCCs);
	
	/*Extend the ppBSCCs array length, we say ppNewBSCCs[0]+2 because we
	so far have ppBSCCs[0]+1 elements in it and we need one extra*/
	ppNewBSCCs = (int **) realloc( ppNewBSCCs, ( lid + 1 ) * sizeof( int *) );
	/*Allocate memory to store the id and the number of nodes of the BSCC*/
	ppNewBSCCs[lid] = (int *) calloc( 2 , sizeof( int ) );
	/*Increase the number of found BSCCs*/
	ppNewBSCCs[ lid ][0] = ++bscc_counter;
	ppNewBSCCs[ lid ][1] = 0;
}

/**
* increases the counter of nodes of the current BSCC
*/
static inline void addBSCCNode()
{
	ppNewBSCCs[ getBSCCCount(ppNewBSCCs) ][1]++;
}

/**
* This function checks if the BSCC consist of 1 node and if yes then
* it stores it's id in the ppNewBSCCs[ppNewBSCCs[0]][2] element
*/
static inline checkForSingleNode(int w)
{
	int lid = getBSCCCount(ppNewBSCCs);
	if( ppNewBSCCs[ lid ][1] == 1 ) {
		/*Add new element*/
		ppNewBSCCs[ lid ]=(int *)realloc(ppNewBSCCs[lid],3*sizeof(int));
		/*Store value*/
		ppNewBSCCs[lid][2] = w;
	}
}

/**************THE BSCC SEARCH PROCEDURES**************************************/

/**
* This method indicates if the w belongs to the component or not and if
* there exists a path from w to some component or not.
* @param w the node to be tested
* @return TRUE if w is in a component or there is a path from w to some component.
*/
static inline BOOL isInComponent(int w)
{
	return ( get_bit_val(pInComponentStates, w) ?  TRUE : FALSE );
}

/**
* This procedure sets the corresponding value to the v element of the
	*pInComponentStates
* bit set. This is used to mark the component as belonging to some state.
*/
static inline void setInComponent(int v, const BITSET_BLOCK_TYPE bit)
{
	set_bit_val(pInComponentStates, v, bit);
}

/**
* This method returns the next after *idx element from the row w. The idx value is then updated.
* @param w the row number
* @param num the number of previously retrieved non zerro element from a row
* @return the value or -1 if all elements have beed processed
*/
static inline int getNextInARow(int w , int * num)
{
	int result = -1;
	int idx = *num;
	BOOL isDiag = pStateSpace->val[w].diag != 0;
	const total = pStateSpace->ncols[w] + (isDiag? 1:0);
	
	/*If there are some unretrieved elements*/
	if ( total > idx ) {
		/*If there is a diagonal element treat it first
		just because it is stored differently*/
		if ( ( idx == 0 ) && isDiag ) {
			/*There is a non zero diagonal element*/
			result = w;
		} else {
			/*Retrieve the rest non zero elements*/
			result = pStateSpace->val[w].col[(isDiag?idx-1:idx)];
		}
		*num = *num + 1;
	}
	
	return result;
}

/**
* This method retrieves a BSCC from the stack.
* @param *cur_stack the pointer to the stack the BSCC should be read
* @param v the root node of the BSCC
* NOTE: this function uses the global int pointer bscc_stack
*/
static void getBSCCFromStack(int v)
{
	int w;
	addBSCCToTheList();
	do {
		w = popStack(bscc_stack);
		setInComponent(w, BIT_ON);
		pBSCCs[w] = bscc_counter;
		addBSCCNode();
	}while(w != v);
	
	checkForSingleNode(w);
}

/**
* This recursive procedure is used to pass through the successive nodes
* in order to detect all the Bottom Strongly Connected Components
* containing the given node. If there is a path from i to some other
* component (SCC) then the procedure exits.
* @param v the initial node
* @param *bscc_stack the stack an eventually found bscc is stored on
* NOTE: this function uses the global int pointer bscc_stack
*/
static void visit_rec(int v)
{
	int w, num=0, initial_root = dfs_order++;
	
	setRoot(v,initial_root);
	setInComponent(v, BIT_OFF);
	setVisited(v);
	bscc_stack = pushStack(bscc_stack, v);
	
	/*Iterate through all the successive nodes*/
	while( ( w = getNextInARow(v, &num) ) != -1 ) {
		if( isNotVisited(w) ) {
			/*Start recursion*/
			visit_rec(w);
			/*Check if we need to exit search*/
			if( bGlobalSkip ) break;
		}
		if( ! isInComponent(w)) {
			setRoot(v, MIN(getRoot(v), getRoot(w)));
		} else {
			/*There is a way from v to some component to which w belongs*/
			/*So v can not be a part of BSCC, thus skip.*/
			bGlobalSkip = TRUE;
			/*setInComponent(v, BIT_ON);*/
			
			/*The bscc_stack pointer might have changed due to the stack
			reallocation, therefore we have to reflect this change in the
			stack array here, since it is not going to be changed while
			this recursion any more*/
			stackarray->stackp[0] = bscc_stack;
			break;
		}
	}
	/*If we did not meet any component yet then it means that there
	can be a BSCC in the stack*/
	if( ! bGlobalSkip ) {
		if ( getRoot(v) == initial_root ) {
			/*Found a BSCC let's get it from the stack.*/
			getBSCCFromStack(v);
		}
	}
}

/**
* This procedure processes the calculations for the nodes in path_stack needed
* for recognizing BSCCs and in case of a BSCC found also gets it from the
* bscc_stack
* @param *path_stack the stack of nodes to do calculations for
*/
static inline void node_calculations(int *path_stack)
{
	int v, w, v_initial_root, num=0;
	v = popStack(path_stack);
	v_initial_root = getRoot(v);
	/*Iterate through all the successive nodes*/
	while( ( w = getNextInARow(v, &num) ) != -1 ) {
		if( ! isInComponent(w)) {
			setRoot(v, MIN(getRoot(v), getRoot(w)));
		} else {
			/*There is a way from v to some component to which w belongs*/
			/*So v can not be a part of BSCC, thus skip.*/
			bGlobalSkip = TRUE;
			/*setInComponent(v, BIT_ON);*/
			break;
		}
	}
	/*If we did not meet any component yet then it means that there
	can be a BSCC in the stack*/
	if( ! bGlobalSkip ) {
		if ( getRoot(v) == v_initial_root ) {
			/*Found a BSCC let's get it from the stack.*/
			getBSCCFromStack(v);
		}
	}
}

/**
* This procedure passes through the successive nodes (depth-first-search)
* in order to detect all the Bottom Strongly Connected Components
* containing the given node. If there is a path from v to some other
* component (SCC) then the procedure exits.
* @param v the initial node
* NOTE: this function uses the global int pointer bscc_stack
*/
static void visit_non_rec(int v)
{
	int w = 0, v_initial_root = 0, succ_counter=0, num=0, initial_root = dfs_order;
	int* dfs_stack = stackarray->stackp[1];
	/* Stores tuples of the form (node, number of node's successors) */
	int* path_stack = stackarray->stackp[2];
	
	/* Push starting node on dfs stack */
	dfs_stack = pushStack(dfs_stack, v);
	
	/* While dfs stack not empty, process with next element on stack */
	while( ( ( v = popStack(dfs_stack) ) != EMPTY_STACK ) && (!bGlobalSkip)) {
		if( ( w = popStack(path_stack) ) != EMPTY_STACK ) {
			/* Decrease successor counter of predecessor */
			path_stack = pushStack(path_stack, --w);
		}
		
		initial_root++;
		setRoot(v,initial_root);
		setInComponent(v, BIT_OFF);
		setVisited(v);
		/* Push v on bscc stack */
		bscc_stack = pushStack(bscc_stack, v);
		succ_counter = 0;
		
		/*Iterate through all the successive nodes*/
		while( ( w = getNextInARow(v, &num) ) != -1 ) {
			if( isNotVisited(w) ) {
				/*Push successive node on dfs-stack*/
				dfs_stack = pushStack(dfs_stack, w);
				/* A non-visited successor found */
				succ_counter++;
			}
		}
		/* Push a tuple of node and number of node's successors on the path stack*/
		path_stack = pushStackTuple(path_stack, v, succ_counter);
		num = 0;
		
		/* As long as all successive nodes have been visited, retrieve elements from
		path stack and do bscc calculations for this nodes */
		while( ((succ_counter = popStack(path_stack)) != EMPTY_STACK ) && (!bGlobalSkip)) {
			/* All successors have been visited */
			if (succ_counter == 0) {
				node_calculations(path_stack);
			} else {
				/*At least one non-visited successor is found,
				process dfs search with next element */
				path_stack = pushStack(path_stack, succ_counter);
				break;
			}
		}
	}
	/*Clean stacks */
	cleanStack(path_stack);
	cleanStack(dfs_stack);
	
	/*The stack pointers might have changed, therefore we
	have to update the stack array*/
	stackarray->stackp[2] = path_stack;
	stackarray->stackp[1] = dfs_stack;
}


/**
* This function returns the list of ids for a newly found BSCCs
* @param pBSCCsHolder the work structure to store all required data for
*						BSCCs search for a particular state space
* @param pStates the set of states
* @return the two dimensional array (result[][])
* result[1][0] the id of a newly found bsccs,
* result[1][1] the number of elements in a bscc
* result[1][2] the id of the node if result[1][1] = 1
*/
int ** getNewBSCCs(TBSCCs * pBSCCsHolder, bitset *pStates)
{
	const int size = pBSCCsHolder->size;
	int i, elem;
	
	/* Get the BSCC method from the runtime.c */
	const int method = get_method_bscc();
	
	/*Define the function pointer that will be assigned
	to the proper VISIT procedure later*/
	void (*visit)(int) = NULL;
	
	/*Init the BSCCs list*/
	initBSCCsList();
	
	/*Allocate the root array;*/
	initRoot(size);
	
	/*Copy data from TBSCC struct to internal variables*/
	pStateSpace = pBSCCsHolder->pStateSpace;
	pVisitedStates = pBSCCsHolder->pVisitedStates;
	pInComponentStates = pBSCCsHolder->pInComponentStates;
	pBSCCs = pBSCCsHolder->pBSCCs;
	bscc_counter = pBSCCsHolder->bscc_counter;
	dfs_order = pBSCCsHolder->dfs_order;
	
	/* Choice for visit_rec or visit_non_rec */
	if( method==REC ) {
		printf("WARNING: Running BSCC search in recursive mode! Segmentation fault\n may occur because of insufficient stack size. If it does, switch\n to non-recursive mode instead.\n");
		/*Allocate the stackarray*/
		stackarray = getNewStackArray(1);
		visit = visit_rec;
	/* Should be non-recursive otherwise */
	} else {
		/*Allocate the stackarray*/
		stackarray = getNewStackArray(3);
		visit = visit_non_rec;
	}
	bscc_stack = stackarray->stackp[0];
	
	/*Pass througn all the states from the *pStates.*/
	for(i=0; i<size; i++) {
		if( get_bit_val(pStates, i) ) {
			if( isNotVisited(i) ) {
				bGlobalSkip = FALSE;
				
				visit(i);
				
				/*Mark all the components in stack as belonged
				to some component*/
				while( ( elem = popStack(bscc_stack) ) != EMPTY_STACK ){
					setInComponent(elem, BIT_ON);
				}
			}
		}
	}
	
	/*The bscc stack pointer might have changed, therefore we
	have to update the stack array*/
	stackarray->stackp[0] = bscc_stack;
	
	/*Copy stack variable values back to TBSCCs*/
	pBSCCsHolder->bscc_counter = bscc_counter;
	pBSCCsHolder->dfs_order = dfs_order;
	
	/*Free the stack memory*/
	freeStackArray(stackarray);
	stackarray = NULL;
	/*The bscc_stack gets freed when the stackarray is freed*/
	bscc_stack = NULL;
	freeRoot();
	
	return getNewBSCCsList();
}
