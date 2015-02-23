/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: core_to_core.c,v $
*	$Revision: 1.38 $
*	$Date: 2007/11/07 13:46:09 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2007.
*	Authors: Ivan Zapreev
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
*	Source description: This is an intermediate interface between
*	the core model checking and the top level model checking.
*/

#include <stdio.h>
#include <stdlib.h>

#include "macro.h"
#include "foxglynn.h"
#include "bitset.h"
#include "sparse.h"
#include "label.h"
#include "runtime.h"
#include "transient.h"
#include "steady.h"
#include "prctl.h"
#include "parser_to_tree.h"
#include "core_to_core.h"

/*******************************************************************/
/****************Model checking atomic formulas*********************/
/*******************************************************************/

/**
* This method creates and returns the bitset filled with ones.
* NOTE: This method resets the probability result no NULL, using set_result_probs(...);
* @return the bitset of the length get_labeller()->ns that
*         contains all states, i.e. every bit in the set is assigned to 1.
*/
static inline bitset * createTrueBitset(){
	labelling *labellin = get_labeller();
	bitset *b = get_new_bitset(labellin->ns);
	fill_bitset_one( b );
	return b;
}

/**
* This method creates and returns the bitset filled with zero.
* NOTE: This method resets the probability result no NULL, using set_result_probs(...);
* @return the bitset of the length get_labeller()->ns that
*         contains no states, i.e. every bit in the set is assigned to 0.
*/
static inline bitset * createFalseBitset(){
	labelling *labellin = get_labeller();
	return get_new_bitset(labellin->ns);
}

/**
* This method returns a set of states that satisfy atomic proposition 'label'.
* NOTE: This method resets the probability result no NULL, using set_result_probs(...);
* @param label: the atomic proposition name.
* @return the set of states satisfying the atomic proposition 'label'.
*/
static inline bitset * getStatesSetByLabel( char * label ){
	//If we just copy reference to the part of
	//labelling function (get_label_bitset) then
	//we will not be able to free bitset memory
	//later in the main loop of mcc.c
	labelling *labellin = get_labeller();
	bitset *b = NULL, *tmp_res = NULL;
	b = get_label_bitset(labellin, label);
	tmp_res = get_new_bitset(labellin->ns);
	if( b != NULL ){
		//Here we basically copy element of b in to a new bitset
		copy_bitset( b, tmp_res );
	}
	
	//If there are no states with this label then
	//just return an empty set.
	return tmp_res;
}

/**
* Does model checking for an atomic formula structure (ATOMIC_SF)
* such as an atomic proposition (AP), TRUE (TT) or FALSE (FF)
* @param before TRUE if the method is called before model checking
*		the subnodes, FALSE otherwise.
*NOTE: We expect the method is called only after the model checking
*	of the subformulas, so we do not check for the value of "before".
* @param pAtomicF the node representing an atomic formula
* @return FALSE, is needed for the doFormulaTreeTraversal method
*/
inline BOOL modelCheckAtomicFormula( BOOL before, PTAtomicF pAtomicF ){
	bitset * pYesBitsetResult = NULL;
	IF_SAFETY( pAtomicF != NULL )
		switch( pAtomicF->atomic_type ){
			case ATOMIC_SF_AP:
				pYesBitsetResult = getStatesSetByLabel(pAtomicF->pName);
				break;
			case ATOMIC_SF_TT:
				pYesBitsetResult = createTrueBitset();
				break;
			case ATOMIC_SF_FF:
				pYesBitsetResult = createFalseBitset();
				break;
			default :
				printf("ERROR: An unknown subtype of ATOMIC_SF.");
				exit(1);
		}
		pAtomicF->type_res.pYesBitsetResult = pYesBitsetResult;
	ELSE_SAFETY
		printf("ERROR: A NULL pointer instead of the PTAtomicF pointer.");
		exit(1);
	ENDIF_SAFETY
	
	return FALSE;
}

/*******************************************************************/
/*************Model checking &&, || and => formulas*****************/
/*******************************************************************/

/**
* This function performs a binary operation on the two bitsets
* @param left the left bitset
* @param right the right bitset
* @param binary_type the binary operation, on of:
*		BINARY_OP_SF_AND, BINARY_OP_SF_OR, BINARY_OP_SF_IMPLIES
* @return the resulting bitset.
*/
static inline bitset * binaryOperationOnBitSets( bitset * pLeftBitset, bitset * pRightBitSet, const int binary_type ){
	bitset * pBitsetResult = NULL;
	bitset * pNotLeftBitSet = NULL;
	IF_SAFETY( ( pLeftBitset != NULL ) && ( pRightBitSet != NULL ) )
		switch(binary_type){
			case BINARY_OP_SF_OR:
				pBitsetResult = or(pLeftBitset, pRightBitSet);
				break;
			case BINARY_OP_SF_AND:
				pBitsetResult = and(pLeftBitset, pRightBitSet);
				break;
			case BINARY_OP_SF_IMPLIES:
				pNotLeftBitSet = not(pLeftBitset);
				pBitsetResult = or(pNotLeftBitSet, pRightBitSet);
				free_bitset(pNotLeftBitSet);
				break;
			default :
				printf("ERROR: An unknown subtype of BINARY_OP_SF.");
				exit(1);
		}
	ELSE_SAFETY
		printf("ERROR: One of the bitsets scheduled for a binary operation is NULL.\n");
		exit(1);
	ENDIF_SAFETY
	return pBitsetResult;
}

/**
* For the || operator we have to unite the YES sets of subformulas to get the YES result.
* To get the NO result we have to intersect the NO sets of subformulas.
* WARNING: We expect that "isSimLeft || isSimRight == TRUE"
*/
static inline processBinaryOrOperatorSim(bitset ** ppYesBitSetResult, bitset ** ppNoBitSetResult,
						bitset *pYesBitsetParamL, bitset *pNoBitsetParamL,
						bitset *pYesBitsetParamR, bitset *pNoBitsetParamR,
						BOOL isSimLeft, BOOL isSimRight ){
	//Get the YES result
	*ppYesBitSetResult = or( pYesBitsetParamL, pYesBitsetParamR );
	//Get the NO result
	if( isSimLeft && isSimRight ){
		//It is just "pNoBitsetParamL && pNoBitsetParamR"
		*ppNoBitSetResult = and( pNoBitsetParamL, pNoBitsetParamR );
	} else {
		if( isSimLeft ){
			//In this case pNoBitsetParamR == NULL and first we construct it
			*ppNoBitSetResult = not( pYesBitsetParamR );
			//Now we compute "pNoBitsetParamL && pNoBitsetParamR"
			and_result( pNoBitsetParamL , *ppNoBitSetResult );
		} else {
			if( isSimRight ){
				//In this case pNoBitsetParamL == NULL and first we construct it
				*ppNoBitSetResult = not( pYesBitsetParamL );
				//Now we compute "pNoBitsetParamR && pNoBitsetParamL"
				and_result( pNoBitsetParamR , *ppNoBitSetResult );
			} else {
				printf("ERROR: Both subformulas of || were not model checked using simulations.\n");
				exit(1);
			}
		}
	}
}

/**
* For the && operator we have to unite the NO sets of subformulas to get the NO result.
* To get the YES result we have to intersect the YES sets of subformulas.
* WARNING: We expect that "isSimLeft || isSimRight == TRUE"
*/
static inline processBinaryAndOperatorSim(bitset ** ppYesBitSetResult, bitset ** ppNoBitSetResult,
						bitset *pYesBitsetParamL, bitset *pNoBitsetParamL,
						bitset *pYesBitsetParamR, bitset *pNoBitsetParamR,
						BOOL isSimLeft, BOOL isSimRight ){
	//Get the YES result
	*ppYesBitSetResult = and( pYesBitsetParamL, pYesBitsetParamR );
	//Get the NO result
	if( isSimLeft && isSimRight ){
		//It is just "pNoBitsetParamL || pNoBitsetParamR"
		*ppNoBitSetResult = or( pNoBitsetParamL, pNoBitsetParamR );
	} else {
		if( isSimLeft ){
			//In this case pNoBitsetParamR == NULL and first we construct it
			*ppNoBitSetResult = not( pYesBitsetParamR );
			//Now we compute "pNoBitsetParamL || pNoBitsetParamR"
			or_result( pNoBitsetParamL , *ppNoBitSetResult );
		} else {
			if( isSimRight ){
				//In this case pNoBitsetParamL == NULL and first we construct it
				*ppNoBitSetResult = not( pYesBitsetParamL );
				//Now we compute "pNoBitsetParamR || pNoBitsetParamL"
				or_result( pNoBitsetParamR , *ppNoBitSetResult );
			} else {
				printf("ERROR: Both subformulas of && were not model checked using simulations.\n");
				exit(1);
			}
		}
	}
}

//A couple of forward definitions
static inline bitset * unaryOperationOnBitSets( bitset * pBitSet, const int unary_type);
static inline unaryOperationOnBitSetsSim( bitset ** ppYesBitSetResult, bitset ** ppNoBitSetResult,
						bitset *pYesBitsetParam, bitset *pNoBitsetParam, const int unary_type );
/**
* For the => operator we just do it as simple as "L => R" ~ "!L || R".
* WARNING: We expect that "isSimLeft || isSimRight == TRUE"
*/
static inline processBinaryImplyOperatorSim(bitset ** ppYesBitSetResult, bitset ** ppNoBitSetResult,
						bitset *pYesBitsetParamL, bitset *pNoBitsetParamL,
						bitset *pYesBitsetParamR, bitset *pNoBitsetParamR,
						BOOL isSimLeft, BOOL isSimRight ){
	//The bitset pointers for storing YES, NO sets of ! L
	bitset * pYesBitsetParamNotL = NULL, * pNoBitsetParamNotL = NULL;
	
	//Compute !L
	if( isSimLeft ){
		unaryOperationOnBitSetsSim( &pYesBitsetParamNotL, &pNoBitsetParamNotL, pYesBitsetParamL,
						pNoBitsetParamL , UNARY_OP_SF_NEGATION );
	} else {
		pYesBitsetParamNotL = unaryOperationOnBitSets( pYesBitsetParamL, UNARY_OP_SF_NEGATION );
	}
	
	//Compute "!L || R"
	processBinaryOrOperatorSim( ppYesBitSetResult, ppNoBitSetResult, pYesBitsetParamNotL, pNoBitsetParamNotL,
					pYesBitsetParamR, pNoBitsetParamR, isSimLeft, isSimRight );
	
	//Free the temporary results of !L
	free_bitset( pYesBitsetParamNotL );
	if( pNoBitsetParamNotL != NULL ){
		free_bitset( pNoBitsetParamNotL );
	}
}

/**
* This method performs binary operations on the results of the simulations.
* WARNING: We expect that "isSimLeft || isSimRight == TRUE"
* @param ppYesBitSetResult the resulting Yes bitset
* @param ppNoBitSetResult the resulting No bitset
* @param pYesBitsetParamL the Yes bitset of the left subformula.
* @param pNoBitsetParamL the No bitset of the left subformula.
* @param pYesBitsetParamR the Yes bitset of the right subformula.
* @param pNoBitsetParamR the No bitset of the right subformula.
* @param binary_type the binary operation, on of:
*		BINARY_OP_SF_AND, BINARY_OP_SF_OR, BINARY_OP_SF_IMPLIES
* @param isSimLeft if the left subformula was simulated.
* @param isSimRight if the right subformula was simulated.
*/
static inline binaryOperationOnBitSetsSim( bitset ** ppYesBitSetResult, bitset ** ppNoBitSetResult,
						bitset *pYesBitsetParamL, bitset *pNoBitsetParamL,
						bitset *pYesBitsetParamR, bitset *pNoBitsetParamR,
						const int binary_type, BOOL isSimLeft, BOOL isSimRight ){
	IF_SAFETY( ( ppYesBitSetResult != NULL ) && ( ppNoBitSetResult != NULL ) )
		switch(binary_type){
			case BINARY_OP_SF_OR:
				processBinaryOrOperatorSim( ppYesBitSetResult, ppNoBitSetResult, pYesBitsetParamL,
								pNoBitsetParamL, pYesBitsetParamR, pNoBitsetParamR,
								isSimLeft, isSimRight );
				break;
			case BINARY_OP_SF_AND:
				processBinaryAndOperatorSim( ppYesBitSetResult, ppNoBitSetResult, pYesBitsetParamL,
								pNoBitsetParamL, pYesBitsetParamR, pNoBitsetParamR,
								isSimLeft, isSimRight );
				break;
			case BINARY_OP_SF_IMPLIES:
				processBinaryImplyOperatorSim( ppYesBitSetResult, ppNoBitSetResult, pYesBitsetParamL,
								pNoBitsetParamL, pYesBitsetParamR, pNoBitsetParamR,
								isSimLeft, isSimRight );
				break;
			default :
				printf("ERROR: An unknown subtype of BINARY_OP_SF.");
				exit(1);
		}
	ELSE_SAFETY
		printf("ERROR: One of the binary-operation return parameters is NULL.\n");
		exit(1);
	ENDIF_SAFETY
}

/**
* This function is used for model checking the binary operator like:
* BINARY_OP_SF_AND, BINARY_OP_SF_OR, BINARY_OP_SF_IMPLIES.
* @param before TRUE if the method is called before model checking
*		the subnodes, FALSE otherwise.
* @param between TRUE if the method is called between model checking of
*		the left and right subformulas
*NOTE: We expect the method is called only after the model checking
*	of the subformulas, so we do not check for "before" and "between".
* @param pBinaryOp the binary operator node
* @return FALSE, is needed for the doFormulaTreeTraversal method
*
* NOTE: Since the subformulas are model checked first
* We do not have to check that pBinaryOp->pSubFormL and
* pBinaryOp->pSubFormR are not NULL.
* NOTE: Because for any formula node the first field is
* of the type TFTypeRes (except for the TFTypeRes itself)
* We can cast the pointer to any formula node into a pointer
* to TFTypeRes in order to access the pYesBitsetResult
*/
inline BOOL modelCheckBinaryOperator( BOOL before, BOOL between, PTBinaryOp pBinaryOp ){
	PTFTypeRes pTypeRes = (PTFTypeRes) pBinaryOp;
	IF_SAFETY( pBinaryOp != NULL )
		PTFTypeRes pFTypeResSubFormL = ( (PTFTypeRes) pBinaryOp->pSubFormL );
		PTFTypeRes pFTypeResSubFormR = ( (PTFTypeRes) pBinaryOp->pSubFormR );
		
		//If the subformula has been simulated then we need a special approach
		if( pTypeRes->doSimBelow ){
			BOOL isSimLeft = ( pFTypeResSubFormL->doSimHere || pFTypeResSubFormL->doSimBelow );
			BOOL isSimRight = ( pFTypeResSubFormR->doSimHere || pFTypeResSubFormR->doSimBelow );
			binaryOperationOnBitSetsSim( &pTypeRes->pYesBitsetResult, &pTypeRes->pNoBitsetResult,
							pFTypeResSubFormL->pYesBitsetResult,
							pFTypeResSubFormL->pNoBitsetResult,
							pFTypeResSubFormR->pYesBitsetResult,
							pFTypeResSubFormR->pNoBitsetResult,
							pBinaryOp->binary_type, isSimLeft, isSimRight );
		} else {
			//If the formula or a subformula was not simulated then it is simple
			//Especially because there is no pTypeResSubF->pNoBitsetResult
			pTypeRes->pYesBitsetResult = binaryOperationOnBitSets( pFTypeResSubFormL->pYesBitsetResult,
										pFTypeResSubFormR->pYesBitsetResult,
										pBinaryOp->binary_type );
		}
	ELSE_SAFETY
		printf("ERROR: A NULL pointer instead of the PTBinaryOp pointer.");
		exit(1);
	ENDIF_SAFETY
	
	return FALSE;
}

/*******************************************************************/
/****************Model checking () and ! formulas*******************/
/*******************************************************************/

/**
* Applyes an unary operation to a bitset ans returns the result
* @param pBitSet the bitset we want to perform operation on.
* @param unary_type the unary operator, one of:
*	 UNARY_OP_SF_NEGATION, UNARY_OP_SF_BRACES.
* @return the resulting bitset, a newly allocated one
*/
static inline bitset * unaryOperationOnBitSets( bitset * pBitSet, const int unary_type){
	bitset * pBitsetResult = NULL;
	
	IF_SAFETY( pBitSet != NULL  )
		switch( unary_type ){
			case UNARY_OP_SF_NEGATION:
				pBitsetResult = not( pBitSet );
				break;
			case UNARY_OP_SF_BRACES:
				pBitsetResult = get_new_bitset( pBitSet->n );
				copy_bitset( pBitSet, pBitsetResult );
				break;
			default :
				printf("ERROR: An unknown subtype of UNARY_OP_SF.");
				exit(1);
		}
	ELSE_SAFETY
		printf("ERROR: A bitset scheduled for an unary operation is NULL.\n");
		exit(1);
	ENDIF_SAFETY
	
	return pBitsetResult;
}

/**
* This method performs unary operations on the results of the simulations.
* 1. For the () operator we just copy the Yes, No bitsets.
* 2. For the ! operator we have to coy and swap the Yes, No bitsets.
* @param ppYesBitSetResult the resulting Yes bitset
* @param ppNoBitSetResult the resulting No bitset
* @param pYesBitsetParam the Yes bitset we want to perform operation on.
* @param pNoBitsetParam the No bitset we want to perform operation on.
* @param unary_type the unary operator, one of:
*	 UNARY_OP_SF_NEGATION, UNARY_OP_SF_BRACES.
*/
static inline unaryOperationOnBitSetsSim( bitset ** ppYesBitSetResult, bitset ** ppNoBitSetResult,
						bitset *pYesBitsetParam, bitset *pNoBitsetParam, const int unary_type ){
	IF_SAFETY( ppYesBitSetResult != NULL && ppNoBitSetResult != NULL )
		IF_SAFETY( ( pYesBitsetParam != NULL ) && ( pNoBitsetParam != NULL ) )
			//Allocate new bitsets
			*ppYesBitSetResult = get_new_bitset( pYesBitsetParam->n );
			*ppNoBitSetResult = get_new_bitset( pNoBitsetParam->n );
			switch( unary_type ){
				case UNARY_OP_SF_NEGATION:
					//Copy the No results to Yes results
					copy_bitset( pNoBitsetParam, *ppYesBitSetResult );
					//Copy the Yes results to No results
					copy_bitset( pYesBitsetParam, *ppNoBitSetResult );
					break;
				case UNARY_OP_SF_BRACES:
					//Copy the Yes results to Yes results
					copy_bitset( pYesBitsetParam, *ppYesBitSetResult );
					//Copy the No results to No results
					copy_bitset( pNoBitsetParam, *ppNoBitSetResult );
					break;
				default :
					printf("ERROR: An unknown subtype of UNARY_OP_SF.");
					exit(1);
			}
		ELSE_SAFETY
			printf("ERROR: One of the bitsets scheduled for an unary operation is NULL.\n");
			exit(1);
		ENDIF_SAFETY
	ELSE_SAFETY
		printf("ERROR: One of the unary-operation return parameters is NULL.\n");
		exit(1);
	ENDIF_SAFETY
}


/**
* This function is used for model checking the unary operator like:
* UNARY_OP_SF_NEGATION, UNARY_OP_SF_BRACES.
* @param before TRUE if the method is called before model checking
*		the subnodes, FALSE otherwise.
*NOTE: We expect the method is called only after the model checking
*	of the subformulas, so we do not check for the value of "before".
* @param pUnaryOp the unary operator node
* @return FALSE, is needed for the doFormulaTreeTraversal method
*/
inline BOOL modelCheckUnaryOperator( BOOL before, PTUnaryOp pUnaryOp ){
	PTFTypeRes pTypeRes = (PTFTypeRes) pUnaryOp;
	
	IF_SAFETY( pUnaryOp != NULL )
		PTFTypeRes pTypeResSubF = (PTFTypeRes) pUnaryOp->pSubForm;
			
		//If the subformula has been simulated then we need a special
		//approach for the negation operator at least
		if( pTypeRes->doSimBelow ){
			unaryOperationOnBitSetsSim( &pTypeRes->pYesBitsetResult, &pTypeRes->pNoBitsetResult,
							pTypeResSubF->pYesBitsetResult, pTypeResSubF->pNoBitsetResult,
							pUnaryOp->unary_type );
		} else {
			//If the formula or a subformula was not simulated then it is simple
			//Especially because there is no pTypeResSubF->pNoBitsetResult
			pTypeRes->pYesBitsetResult = unaryOperationOnBitSets( pTypeResSubF->pYesBitsetResult,
									pUnaryOp->unary_type );
		}
	ELSE_SAFETY
		printf("ERROR: A NULL pointer instead of the PTUnaryOp pointer.");
		exit(1);
	ENDIF_SAFETY
	
	return FALSE;
}

/*******************************************************************/
/***************Model checking S, L and P formulas******************/
/*******************************************************************/
/**
* This is a local method just for sorting the probab./reward result and
* creation of the satisfyability set.
* @param theProbRewardResult the value which needs to sort out.
* @param val_bound_left the values bound (prob./reward)
* @param val_bound_right the values bound (prob./reward)
* @param comparator the comparator, one of 
*/
static inline void sortOutSingleValue(const double theProbRewardResult, const int theStateIndex, const double val_bound_left,
					const double val_bound_right, const int comparator, bitset * pYesBitsetResult){
	switch(comparator){
		case COMPARATOR_SF_GREATER:
			if ( theProbRewardResult > val_bound_left ){
				set_bit_val( pYesBitsetResult, theStateIndex, BIT_ON );
			}
			break;
		case COMPARATOR_SF_GREATER_OR_EQUAL:
			if ( theProbRewardResult >= val_bound_left ){
				set_bit_val( pYesBitsetResult, theStateIndex, BIT_ON );
			}
			break;
		case COMPARATOR_SF_LESS_OR_EQUAL:
			if ( theProbRewardResult <= val_bound_left ){
				set_bit_val( pYesBitsetResult, theStateIndex, BIT_ON );
			}
			break;
		case COMPARATOR_SF_LESS:
			if ( theProbRewardResult < val_bound_left ){
				set_bit_val( pYesBitsetResult, theStateIndex, BIT_ON );
			}
			break;
		case COMPARATOR_SF_REWARD_INTERVAL:
			if ( ( val_bound_left <= theProbRewardResult ) &&
				( theProbRewardResult <= val_bound_right ) ){
				set_bit_val( pYesBitsetResult, theStateIndex, BIT_ON );
			}
			break;
	}
}

/**
* This is a local method just for sorting the probab./reward results and
* creation of the satisfyability set.
* @param pProbRewardResult the array of probabilities
* @param val_bound_left the values bound (prob./reward)
* @param val_bound_right the values bound (prob./reward)
* @param comparator the comparator, one of:
*			
* @param error_bound the error bound that should be takein into account
* @return the set filled with the states 'i' that satisfy:
*	1) COMPARATOR_SF_REWARD_INTERVAL:
*		val_bound_left - error_bound <= pProbRewardResult[i] <= val_bound_right + error_bound
*	2) COMPARATOR_SF_GREATER, COMPARATOR_SF_GREATER_OR_EQUAL:
*		pProbRewardResult[i] > val_bound_left - error_bound,
*		pProbRewardResult[i] >= val_bound_left - error_bound
*	3) COMPARATOR_SF_LESS, COMPARATOR_SF_LESS_OR_EQUAL:
*		pProbRewardResult[i] < val_bound_left + error_bound,
*		pProbRewardResult[i] <= val_bound_left + error_bound
* WARNING: The Qureshi-Sanders Uniformization for Time- and Reward- bounded Until of CSRL
*          Provides us with the entire array of errors for each state, the same should be
*          happening with the Steady-state (Long-run) operator of PCTL, CSL, PRCTL, CSRL.
*          So having just one variable for error_bound here is not enough and this should
*          be fixed some time, Unfortunately we did not care about the error bounds much
*          before, so we have to implement a proper error-bound computation first.
*          Especially for the case of Nested formulas, where the error-bound derivation
*          formulas are just unknown.
*/
static inline bitset * sortOutStatesAccordingToProbs( const double *pProbRewardResult, const double val_bound_left,
							const double val_bound_right, const int comparator, const double error_bound){
	const int size = get_labeller()->ns;
	//TODO: Since probabilistic results are computed with a certain error bound for 
	//the untill and steady-state (long-run) operators of PRCTL, PCTL, CSL and CSRL,
	//we have to check +- error bound bound:
	//1. Unbounded Untill of PRCTL, PCTL, CSL and CSRL.
	//2. Time-interval and Time-bounded Untill of CSL and CSRL.
	//3. Time and reward- bounded until of CSRL. Here though we should find
	//   out what is the error bound for Qureshi-Sanders uniformization and
	//   Tijms-Veldman discretization.
	//4. Long-run operator of PCTL, CSRL and Steady-state of CSL, CSRL.
	
	//Recompute the bounds based on the error_bound
	double val_bound_left_error = 0.0, val_bound_right_error = 0.0;
	if( error_bound != 0 ){
		switch(comparator){
			case COMPARATOR_SF_GREATER:
			case COMPARATOR_SF_GREATER_OR_EQUAL:
				val_bound_left_error = val_bound_left - error_bound;
				break;
			case COMPARATOR_SF_LESS:
			case COMPARATOR_SF_LESS_OR_EQUAL:
				val_bound_left_error = val_bound_left + error_bound;
				break;
			case COMPARATOR_SF_REWARD_INTERVAL:
				val_bound_left_error = val_bound_left - error_bound;
				val_bound_right_error = val_bound_right + error_bound;
				break;
		}
	}
	
	int i = 0;
	bitset * pYesBitsetResult = get_new_bitset( size );
	//Check for the bounds.
	if( error_bound != 0 ){
		for( i=0; i < size; i++ ){
			double theProbRewardResult = pProbRewardResult[i];
			//If the value is 0.0 or 1.0 then it was computed precisely so no
			//error bound should be taken into account
			if( ( theProbRewardResult == 0.0 ) || ( theProbRewardResult == 1.0 ) ){
				sortOutSingleValue( theProbRewardResult, i, val_bound_left, val_bound_right,
							comparator, pYesBitsetResult);
			}else{
				sortOutSingleValue( theProbRewardResult, i, val_bound_left_error, val_bound_right_error,
							comparator, pYesBitsetResult);
			}
		}
	} else {
		//In this case things are simple, we just assume the exact computations
		for( i=0; i < size; i++ ){
			sortOutSingleValue( pProbRewardResult[i], i, val_bound_left, val_bound_right,
						comparator, pYesBitsetResult);
		}
	}
	return pYesBitsetResult;
}

/**
* This function is used for model checking the comparator formula:
* COMPARATOR_SF_LESS, COMPARATOR_SF_LESS_OR_EQUAL,
* COMPARATOR_SF_GREATER, COMPARATOR_SF_GREATER_OR_EQUAL.
* @param before TRUE if the method is called before model checking
*		the subnodes, FALSE otherwise.
*NOTE: We expect the method is called only after the model checking
*	of the subformulas, so we do not check for the value of "before".
* @param pCompStateF the comparator wrapper formula node
* @return FALSE, is needed for the doFormulaTreeTraversal method
*/
inline BOOL modelCheckComparatorFormula( BOOL before, PTCompStateF pCompStateF ){
	PTFTypeRes pFTypeRes = (PTFTypeRes) pCompStateF;
	PTFTypeRes pFTypeResSubForm;
	
	IF_SAFETY( pCompStateF != NULL )
		pFTypeResSubForm = (PTFTypeRes) pCompStateF->unary_op.pSubForm;
		IF_SAFETY( pFTypeResSubForm != NULL )
			if( pFTypeResSubForm->doSimHere ) {
				//In case the subformula was simulated, we actually
				//computed the Yes/No bitsets there, due to the model
				//checking nature therefore not we only should copy
				//them here. and erase the pointers in the subformula
				pFTypeRes->pYesBitsetResult = pFTypeResSubForm->pYesBitsetResult;
				pFTypeRes->pNoBitsetResult = pFTypeResSubForm->pNoBitsetResult;
				pFTypeResSubForm->pYesBitsetResult = NULL;
				pFTypeResSubForm->pNoBitsetResult = NULL;
			}else{
				pFTypeRes->pYesBitsetResult = sortOutStatesAccordingToProbs( pFTypeResSubForm->pProbRewardResult,
											pCompStateF->val_bound_left,
											pCompStateF->val_bound_right,
											pCompStateF->unary_op.unary_type,
											pFTypeResSubForm->error_bound );
			}
		ELSE_SAFETY
			printf("ERROR: The comparator subformula is NULL, unexpected!");
			exit(1);
		ENDIF_SAFETY
	ELSE_SAFETY
		printf("ERROR: A NULL pointer instead of the PTCompStateF pointer.");
		exit(1);
	ENDIF_SAFETY
	
	return FALSE;
}

/*******************************************************************/
/*********Model checking (computing probs) S and L formulas*********/
/*******************************************************************/

//TODO: Since probabilistic results are computed with a certain error bound for 
//the steady-state (long-run) operators of PRCTL, PCTL, CSL and CSRL,
//we have to check +- error bound bound:
//WARNING: Here the provided error bound is not quite correct,
//it is tighter than it should be but at least it is something!
//	S{<=>p}[phi], err = get_error_bound()
//1. In case of ergodic MC we have to take:
//	error_bound = err * | phi |
//2. In case of Non ergodic MC it should be:
//	error_bound = \Sum_j (y_j * err + x_ij * err * | phi_j | + err^2 * | phi_j |)
//Where \Sum_j ( (x_ij +- err)*(y_j +- err * | phi_j |) )
//is the way the probabiulity is computed for the state i.
//	x_ij  - the probability to reach BSCC_j from state i
//	y_j   - the the total steady state probability of phi states in BSCC_j
//	phi_j - the phi states in BSCC_j
/**
* This function is used for computing the probabilities of formulas:
* LONG_STEADY_F_LONG_RUN, LONG_STEADY_F_STEADY_STATE.
* @param before TRUE if the method is called before model checking
*		the subnodes, FALSE otherwise.
*NOTE: We expect the method is called only after the model checking
*	of the subformulas, so we do not check for the value of "before".
* @param pLongSteadyF the long-run/steady-state probability formula
* @return FALSE, is needed for the doFormulaTreeTraversal method
*/
inline BOOL modelCheckLongSteadyFormula( BOOL before, PTLongSteadyF pLongSteadyF ){
	PTFTypeRes pFTypeRes = (PTFTypeRes) pLongSteadyF;
	//WARNING: This works until PTLongSteadyF has TUnaryOp as a first field
	PTUnaryOp pUnaryOp = (PTUnaryOp) pLongSteadyF;
	
	IF_SAFETY( isRunMode(PCTL_MODE) || isRunMode(PRCTL_MODE) || isRunMode(CSL_MODE) || isRunMode(CSRL_MODE) )
		IF_SAFETY( pLongSteadyF != NULL )
			pFTypeRes->pProbRewardResult = steady( ( (PTFTypeRes) pUnaryOp->pSubForm)->pYesBitsetResult);
			//WARNING: This is a temporary solution, has to be fixed later
			//Also the steady(...) method should have the error bound as a parameter.
			pFTypeRes->error_bound = get_error_bound();
			pFTypeRes->prob_result_size = get_labeller()->ns;
		ELSE_SAFETY
			printf("ERROR: A NULL pointer instead of the PTLongSteadyF pointer.");
			exit(1);
		ENDIF_SAFETY
	ELSE_SAFETY
		printf("ERROR: Steady-state formula S (long-run formula L) is only valid in CSL and CSRL (PCTL and PRCTL) mode.\n");
	ENDIF_SAFETY
	return FALSE;
}

/*******************************************************************/
/*********Model checking (computing probs) E, C, Y formulas*********/
/*******************************************************************/

//TODO: It looks like it would be a good idea to split this into a Comparator
//like operator plus the E,C,Y formulas wothout the reward bounds checking
//Then we could process these like S, L and U operators. Unfortunately I have
//no time to fix it now.
//TODO: The Y,C,Y rewards are computed with some error bound, which is not taken
//into account then the reward constraints are applied!
/**
* Universal method for E,C and Y formula
* NOTE: It stores the probability result, if any, in runtime.c using set_result_probs(...)
* NOTE: the pBitset is freed here
* @param type the type which defines what formula it is: 
* @param pBitset the bitset of states phi if 'X[t_bound_1,t_bound_2] phi' is checked
* @param epoch the epoch, for the long run expected reward rate should be set to 0 (ZERO)
* @param p_prob_result_size the pointer to the variable that willl store the size of the
*				returned vector;
* @return the bitset of states that satisfy the formula
*/
static inline double * getEECY(const int TYPE, bitset * pBitset, int epoch,  int * p_prob_result_size){
	const int size = get_labeller()->ns;
	double *pProbRewardResult = NULL;
	
	if( isRunMode(PRCTL_MODE) )
	{
		switch(TYPE){
			case PURE_REWARD_SF_EXPECTED_RR:
				pProbRewardResult = ef(epoch, pBitset);
				break;
			case PURE_REWARD_SF_INSTANT_R:
				pProbRewardResult = cf(epoch, pBitset);
				break;
			case PURE_REWARD_SF_EXPECTED_AR:
				pProbRewardResult = yf(epoch, pBitset);
				break;
			default:
				printf("ERROR: Undefined EECY type. This should not be happening!\n");
				exit(1);
		}
		
	}else{
		printf("ERROR: The expected reward rate formula E, instantaneous reward formula C and expected accumulated reward formula Y are valid only in PRCTL mode.\n");
		pProbRewardResult = (double *)calloc( size, sizeof(double) );
	}
	
	//Assign the size of pProbRewardResult
	*p_prob_result_size = size;
	
	return pProbRewardResult;
}

/**
* This function is used for model checking the formulas:
* PURE_REWARD_SF_EXPECTED_RR, PURE_REWARD_SF_INSTANT_R,
* PURE_REWARD_SF_EXPECTED_AR.
* @param before TRUE if the method is called before model checking
*		the subnodes, FALSE otherwise.
*NOTE: We expect the method is called only after the model checking
*	of the subformulas, so we do not check for the value of "before".
* @param pPureRewardF the pure-reward state formula: E, C, Y
* @return FALSE, is needed for the doFormulaTreeTraversal method
*/
inline BOOL modelCheckPureRewardFormula( BOOL before, PTPureRewardF pPureRewardF ){
	PTFTypeRes pFTypeRes = (PTFTypeRes) pPureRewardF;
	IF_SAFETY( pPureRewardF != NULL )
		pFTypeRes->pProbRewardResult = getEECY( pPureRewardF->unary_op.unary_type,
						( (PTFTypeRes) pPureRewardF->unary_op.pSubForm)->pYesBitsetResult,
						pPureRewardF->time, &(pFTypeRes->prob_result_size) );
		//TODO: There has to be a proper error bound assigned in
		//the future, and may be not at this point but some time earlier.
		pFTypeRes->error_bound = get_error_bound();
	ELSE_SAFETY
		printf("ERROR: A NULL pointer instead of the PTPureRewardF pointer.");
		exit(1);
	ENDIF_SAFETY
	
	return FALSE;
}

/*******************************************************************/
/*************Model checking (computing probs) X formula************/
/*******************************************************************/

/**
* This function calls model-checking algorithm for the next formula
* @param pBitset the bitset of states phi if 'X phi' is checked
* @return the array of probabilities
*/
static inline double * getNextProbability(bitset * pBitset){
	double * result = NULL;
	
	if( isRunMode(PCTL_MODE) || isRunMode(CSL_MODE) || isRunMode(PRCTL_MODE) || isRunMode(CSRL_MODE) ){
		result = next(TIME_UNBOUNDED_FORM, pBitset, 0.0, 0.0);
	}else{
		printf("ERROR: Next formula X is valid only in PCTL, CSL, PRCTL and CSRL mode.\n");
		result = (double *)calloc( get_state_space()->rows, sizeof(double) );
	}
	
	return result;
}

/**
* This function calls model-checking algorithm for the time-bounded next formula
* @param pBitset the bitset of states phi if 'X[t_bound_1,t_bound_2] phi' is checked
* @param t_bound_1 lower time bound
* @param t_bound_2 upper time bound
* @return the array of probabilities
*/
static inline double * getTimeIntervalNextProbability(bitset * pBitset, double t_bound_1, double t_bound_2){
	double * result = NULL;
	
	if( isRunMode(CSL_MODE) || isRunMode(CSRL_MODE) ){
		result = next(TIME_INTERVAL_FORM, pBitset, t_bound_1, t_bound_2);
	}else{
		printf("ERROR: Time-bounded next formula X is valid only in CSL and CSRL mode.\n");
		result = (double *)calloc( get_state_space()->rows, sizeof(double));
	}
	
	return result;
}

/**
* This function calls model-checking algorithm for the time- and reward-bounded next formula
* @param pBitset the bitset of states phi if 'X[t_bound_1,t_bound_2] phi' is checked
* @param t_bound_1 lower time bound
* @param t_bound_2 upper time bound
* @param r_bound_1 lower reward bound
* @param r_bound_2 upper reward bound
* @return the array of probabilities
*/
static inline double * getTimeAndRewardBoundedNextProbability(bitset * pBitset, double t_bound_1, double t_bound_2,
								double r_bound_1, double r_bound_2){
	double * result = NULL;
	
	if( isRunMode(CSRL_MODE) ){
		result = next_rewards(pBitset, t_bound_1, t_bound_2, r_bound_1, r_bound_2);
	}else{
		printf("ERROR: Time- and reward-bounded next formula X is valid only in CSRL mode.\n");
		result = (double *)calloc( get_state_space()->rows, sizeof(double));
	}
	
	return result;
}

/**
* This function is used for model checking the formulas:
* NEXT_PF_UNB, NEXT_PF_TIME, NEXT_PF_TIME_REWARD
* @param before TRUE if the method is called before model checking
*		the subnodes, FALSE otherwise.
*NOTE: We expect the method is called only after the model checking
*	of the subformulas, so we do not check for the value of "before".
* @param pNextF the X formula tree
* @return FALSE, is needed for the doFormulaTreeTraversal method
*/
inline BOOL modelCheckNextFormula( BOOL before, PTNextF pNextF ){
	PTFTypeRes pFTypeRes = (PTFTypeRes) pNextF;
	bitset * pYesBitsetResultSubForm = NULL;
	double * pProbRewardResult = NULL;
	
	IF_SAFETY( pNextF != NULL )
		pYesBitsetResultSubForm = ( (PTFTypeRes) pNextF->unary_op.pSubForm)->pYesBitsetResult;
		switch( pNextF->unary_op.unary_type ){
			case NEXT_PF_UNB:
				pProbRewardResult = getNextProbability( pYesBitsetResultSubForm );
				break;
			case NEXT_PF_TIME:
				pProbRewardResult = getTimeIntervalNextProbability( pYesBitsetResultSubForm,
										pNextF->left_time_bound,
										pNextF->right_time_bound );
				break;
			case NEXT_PF_TIME_REWARD:
				pProbRewardResult = getTimeAndRewardBoundedNextProbability( pYesBitsetResultSubForm,
											pNextF->left_time_bound,
											pNextF->right_time_bound,
											pNextF->left_reward_bound,
											pNextF->right_reward_bound );
				break;
			default:
				printf("ERROR: An unknown type '%d' of the Next operator.\n", pNextF->unary_op.unary_type);
				exit(1);
		}
		pFTypeRes->pProbRewardResult = pProbRewardResult;
		pFTypeRes->prob_result_size = get_labeller()->ns;
	ELSE_SAFETY
		printf("ERROR: A NULL pointer instead of the PTNextF pointer.");
		exit(1);
	ENDIF_SAFETY
	
	return FALSE;
}

/*******************************************************************/
/************Model checking (computing probs) U formulas************/
/*******************************************************************/

/**
* This function calls model-checking algorithm for the unbounded until
* @param pPhiBitset the phi states bitset
* @param pPsiBitset the psi states bitset
*
* Below we have the return parameters
*
* @param ppProbRewardResult the array of probabilities
* @param prob_result_size the size of pProbRewardResult
* @param error_bound the error bound for the results in pProbRewardResult
*/
static inline void getUnboundedUntilProbability(bitset * pPhiBitset, bitset * pPsiBitset, double ** ppProbRewardResult,
						int * prob_result_size, double * error_bound){
	double * result = NULL;
	
	if( isRunMode(CSL_MODE) || isRunMode(PCTL_MODE) || isRunMode(PRCTL_MODE) || isRunMode(CSRL_MODE) ){
		result = until(TIME_UNBOUNDED_FORM, pPhiBitset, pPsiBitset, 0.0, 0.0, FALSE);
	}else{
		printf("ERROR: Unbounded until formula U is valid only in PCTL, CSL, PRCTL and CSRL mode.\n");
		result = (double *)calloc( get_state_space()->rows , sizeof(double));
	}
	
	//Assign the results
	*ppProbRewardResult = result;
	*prob_result_size = get_labeller()->ns;
	//TODO: There has to be a proper error bound assigned in
	//the future, and may be not at this point but some time earlier.
	*error_bound = get_error_bound();
}

/**
* This function calls model-checking algorithm for the time-bounded until
* @param pPhiBitset the phi states bitset
* @param pPsiBitset the psi states bitset
* @param t_bound_1 lower time bound
* @param t_bound_2 upper time bound
*
* Below we have the return parameters
*
* @param ppProbRewardResult the array of probabilities
* @param prob_result_size the size of pProbRewardResult
* @param error_bound the error bound for the results in pProbRewardResult
*/
static inline double * getTimeIntervalUntilProbability(bitset * pPhiBitset, bitset * pPsiBitset,
							double t_bound_1, double t_bound_2, double ** ppProbRewardResult,
							int * prob_result_size, double * error_bound){
	double * result = NULL;
	
	if( isRunMode(CSL_MODE) || isRunMode(PCTL_MODE) || isRunMode(PRCTL_MODE) || isRunMode(CSRL_MODE) ){
		result = until(TIME_INTERVAL_FORM, pPhiBitset, pPsiBitset, t_bound_1, t_bound_2, FALSE);
	}else{
		printf("ERROR: Time-bounded until formula U is valid only in PCTL, CSL, PRCTL and CSRL mode.\n");
		result = (double *)calloc( get_state_space()->rows , sizeof(double));
	}
	
	//Assign the results
	*ppProbRewardResult = result;
	*prob_result_size = get_labeller()->ns;
	//TODO: There has to be a proper error bound assigned in
	//the future, and may be not at this point but some time earlier.
	*error_bound = get_error_bound();
}

/**
* This function calls model-checking algorithm for the time- and reward-bounded until
* @param pPhiBitset the phi states bitset
* @param pPsiBitset the psi states bitset
* @param t_bound_1 lower time bound
* @param t_bound_2 upper time bound
* @param r_bound_1 lower reward bound
* @param r_bound_2 upper reward bound
*
* Below we have the return parameters
*
* @param ppProbRewardResult the array of probabilities
* @param prob_result_size the size of pProbRewardResult
* @param error_bound the error bound for the results in pProbRewardResult
*/
static inline double * getTimeAndRewardBoundedUntilProbability(bitset * pPhiBitset, bitset * pPsiBitset,
								double t_bound_1, double t_bound_2,
								double r_bound_1, double r_bound_2,
								double ** ppProbRewardResult,
								int * prob_result_size, double * error_bound){
	double * result = NULL;
	
	if( isRunMode(PRCTL_MODE) || isRunMode(CSRL_MODE) ){
		result = until_rewards(pPhiBitset, pPsiBitset, t_bound_1, t_bound_2, r_bound_1, r_bound_2, FALSE);
	}else{
		printf("ERROR: Time- and reward-bounded until formula is valid only in PRCTL and CSRL mode.\n");
		result = (double *)calloc( get_state_space()->rows, sizeof(double) );
	}
	
	//Assign the results
	*ppProbRewardResult = result;
	*prob_result_size = get_labeller()->ns;
	//TODO: There has to be a proper error bound assigned in
	//the future, and may be not at this point but some time earlier.
	*error_bound = get_error_bound();
}

/**
* Invokesthe simulation procedure for the unbounded until operator: "Phi U Psi"
* @param pPhiBitSet the Phi states
* @param pPsiBitSet the Psi states
* @param pUntilF the unbounded-until operator tree node with the needed parameters
*			Note that the results will be stored in this node
*/
static inline void simulateUnboundedUntil( const bitset * pPhiBitSet, const bitset *pPsiBitSet, PTUntilF pUntilF ){
	PTFTypeRes pFTypeRes = (PTFTypeRes) pUntilF;
	
	sparse * pStateSpace = get_state_space();
	const double * pCTMCRowSums = get_row_sums();
	const double confidence = pFTypeRes->confidence;
	
	bitset ** ppYesBitsetResult = &pFTypeRes->pYesBitsetResult;
	bitset ** ppNoBitsetResult = &pFTypeRes->pNoBitsetResult;
	double ** ppProbCILeftBorder = &pFTypeRes->pProbCILeftBorder;
	double ** ppProbCIRightBorder = &pFTypeRes->pProbCIRightBorder;
	int * pResultSize = &pFTypeRes->prob_result_size;
	
	const int comparator = pUntilF->pCompStateF->unary_op.unary_type;
	const double prob_bound = pUntilF->pCompStateF->val_bound_left;
	
	const int initial_state = pFTypeRes->initial_state;
	const BOOL isSimOneInitState = pFTypeRes->isSimOneInitState;
	
	IF_SAFETY( isRunMode(CSL_MODE) || isRunMode(CSRL_MODE) )
		modelCheckUnboundedUntilCTMC( pStateSpace, pCTMCRowSums, confidence, pPhiBitSet, pPsiBitSet, ppYesBitsetResult,
						ppNoBitsetResult, ppProbCILeftBorder, ppProbCIRightBorder, pResultSize,
						comparator, prob_bound, initial_state, isSimOneInitState );
	ELSE_SAFETY
		printf("ERROR: The Unbounded until formula can be simulated only in CSL and CSRL mode.\n");
		exit(1);
	ENDIF_SAFETY
}

	
//TODO: Since some probabilistic results are computed with a certain error
//bound for the untill operators of PRCTL, PCTL, CSL and CSRL, we have
//to check +- error bound bound:
//1. Unbounded Untill of PRCTL, PCTL, CSL and CSRL.
//2. Time-interval and Time-bounded Untill of CSL and CSRL.
//3. Time and reward- bounded until of CSRL. Here though we should find
//   out what is the error bound for Qureshi-Sanders uniformization and
//   Tijms-Veldman discretization.
//WARNING: Here the provided error bound is not quite correct,
//it is tighter than it should be but at least it is something!
//See comments for the
//	sortOutStatesAccordingToProbs(...)
//method as well.
/**
* This function is used for model checking the formulas:
* UNTIL_PF_UNB, UNTIL_PF_TIME, UNTIL_PF_TIME_REWARD
* @param before TRUE if the method is called before model checking
*		the subnodes, FALSE otherwise.
* @param between TRUE if the method is called between model checking of
*		the left and right subformulas
*NOTE: We expect the method is called only after the model checking
*	of the subformulas, so we do not check for "before" and "between".
* @param pUntilF the U formula tree
* @return FALSE, is needed for the doFormulaTreeTraversal method
*/
inline BOOL modelCheckUntilFormula( BOOL before, BOOL between, PTUntilF pUntilF ){
	PTFTypeRes pFTypeRes = (PTFTypeRes) pUntilF;
	bitset * pYesBitsetResultSubFormL = NULL, * pYesBitsetResultSubFormR = NULL;
	
	IF_SAFETY( pUntilF != NULL )
		pYesBitsetResultSubFormL = ( (PTFTypeRes) pUntilF->binary_op.pSubFormL)->pYesBitsetResult;
		pYesBitsetResultSubFormR = ( (PTFTypeRes) pUntilF->binary_op.pSubFormR)->pYesBitsetResult;
		switch( pUntilF->binary_op.binary_type ){
			case UNTIL_PF_UNB:
				if( pFTypeRes->doSimHere ){
					simulateUnboundedUntil( pYesBitsetResultSubFormL, pYesBitsetResultSubFormR, pUntilF );
				} else {
					getUnboundedUntilProbability( pYesBitsetResultSubFormL,
									pYesBitsetResultSubFormR,
									& pFTypeRes->pProbRewardResult,
									& pFTypeRes->prob_result_size,
									& pFTypeRes->error_bound );
				}
				break;
			case UNTIL_PF_TIME:
				getTimeIntervalUntilProbability( pYesBitsetResultSubFormL,
								pYesBitsetResultSubFormR,
								pUntilF->left_time_bound,
								pUntilF->right_time_bound,
								& pFTypeRes->pProbRewardResult,
								& pFTypeRes->prob_result_size,
								& pFTypeRes->error_bound );
				break;
			case UNTIL_PF_TIME_REWARD:
				getTimeAndRewardBoundedUntilProbability( pYesBitsetResultSubFormL,
									pYesBitsetResultSubFormR,
									pUntilF->left_time_bound,
									pUntilF->right_time_bound,
									pUntilF->left_reward_bound,
									pUntilF->right_reward_bound,
	 								& pFTypeRes->pProbRewardResult,
									& pFTypeRes->prob_result_size,
									& pFTypeRes->error_bound );
				break;
			default:
				printf("ERROR: An unknown type '%d' of the Until operator.\n", pUntilF->binary_op.binary_type);
				exit(1);
		}
	ELSE_SAFETY
		printf("ERROR: A NULL pointer instead of the PTUntilF pointer.");
		exit(1);
	ENDIF_SAFETY
	
	return FALSE;
}
