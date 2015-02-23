/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: parser_to_tree.c,v $
*	$Revision: 1.49 $
*	$Date: 2007/10/14 16:04:14 $
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
*	Source description: This is an intermediate interface that allows to create the
*	model-checking formula tree.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "macro.h"
#include "runtime.h"
#include "parser_to_tree.h"

static void initializeTFTypeRes( PTFTypeRes pFTypeRes, int formula_type ){
	pFTypeRes->formula_type = formula_type;
	pFTypeRes->pCharFormulaString = NULL;
	pFTypeRes->prob_result_size = 0;
	
	pFTypeRes->initial_state = -1;
	pFTypeRes->isSimOneInitState = FALSE;
	pFTypeRes->doSimHere = FALSE;
	pFTypeRes->doSimBelow = FALSE;
	pFTypeRes->confidence = 0.0;
	pFTypeRes->pYesBitsetResult = NULL;
	pFTypeRes->pNoBitsetResult = NULL;
	pFTypeRes->pProbCILeftBorder = NULL;
	pFTypeRes->pProbCIRightBorder = NULL;
	
	pFTypeRes->error_bound = 0.0;
	pFTypeRes->pErrorBound = NULL;
	pFTypeRes->pProbRewardResult = NULL;
}

static void initializeTUnaryOp( PTUnaryOp pUnaryOp, int formula_type, int unary_type, void * pSubForm ){
	initializeTFTypeRes( &(pUnaryOp->type_res), formula_type);
	pUnaryOp->unary_type = unary_type;
	pUnaryOp->pSubForm = pSubForm;
}

static void initializeTBinaryOp( PTBinaryOp pBinaryOp, int formula_type, int binary_type, void * pSubFormL, void * pSubFormR ){
	initializeTFTypeRes( &(pBinaryOp->type_res), formula_type);
	pBinaryOp->binary_type = binary_type;
	pBinaryOp->pSubFormL = pSubFormL;
	pBinaryOp->pSubFormR = pSubFormR;
}

/**
* Creates an atomic formula structure (ATOMIC_SF) such as an atomic
* proposition (AP), TRUE (TT) or FALSE (FF)
* @param label contain one of: AP, NAME_FALSE, NAME_TRUE
* @param atomic_type one of: ATOMIC_SF_AP, ATOMIC_SF_TT, ATOMIC_SF_FF
* @return returns the pointer to the newly created TAtomicF node casted to (void *)
*/
void * allocateAtomicFormula(char * pLabel, const int atomic_type){
	//plus 1 in order to preserve the terminating byte
	int name_length = strlen(pLabel) + 1;
	PTAtomicF pAtomicF = (PTAtomicF) calloc(1, sizeof(TAtomicF));
	
	initializeTFTypeRes( &(pAtomicF->type_res), ATOMIC_SF);
	pAtomicF->atomic_type = atomic_type;
	
	//WARNING: We can not just assign a pointer here, because this pLabel
	//pointer comes from somewhere else, namely from the lex and yacc code
	pAtomicF->pName = calloc(name_length, sizeof(char));
	strcpy(pAtomicF->pName, pLabel);
	
	return (void *) pAtomicF;
}

/**
* Creates a "unary operation on a subformula" structure (UNARY_OP_SF)
* such as braces (pSubForm) or negation !pSubForm
* @param pSubForm the subformula for the unary operator
* @param unary_type one of: UNARY_OP_SF_NEGATION, UNARY_OP_SF_BRACES
* @return returns the pointer to the newly created TUnaryOp node casted to (void *)
*/
void * allocateUnaryOperatorFormula(void * pSubForm, const int unary_type){
	PTUnaryOp pUnaryPoF = (PTUnaryOp) calloc(1, sizeof(TUnaryOp));
	
	initializeTUnaryOp( pUnaryPoF, UNARY_OP_SF, unary_type, pSubForm );
	
	return (void *) pUnaryPoF;
}

/**
* Creates a "binary operation on a subformula" structure (BINARY_OP_SF)
* such as and pSubFormL&&pSubFormR, or pSubFormL||pSubFormR, implication pSubFormL=>pSubFormR
* @param pSubFormL the left subformula for the binary operator
* @param pSubFormR the right subformula for the binary operator
* @param unary_type one of: BINARY_OP_SF_IMPLIES, BINARY_OP_SF_AND, BINARY_OP_SF_OR
* @return returns the pointer to the newly created TUnaryOp node casted to (void *)
*/
void * allocateBinaryOperatorFormula(void * pSubFormL, void * pSubFormR, const int binary_type){
	PTBinaryOp pBinaryPoF = (PTBinaryOp) calloc(1, sizeof(TBinaryOp));
	
	initializeTBinaryOp( pBinaryPoF, BINARY_OP_SF, binary_type, pSubFormL, pSubFormR );
	
	return (void *) pBinaryPoF;
}

/**
* Creates a simple comparator formula (COMPARATOR_SF), a wrapper for the : S, L, P operators
* @param pSubForm the subformula for the simple formula
* @param val_bound_left the values bound (prob./reward)
* @param val_bound_right the values bound (prob./reward)
* @param unary_type one of: COMPARATOR_SF_LESS, COMPARATOR_SF_LESS_OR_EQUAL,
* 				COMPARATOR_SF_GREATER, COMPARATOR_SF_GREATER_OR_EQUAL,
*				COMPARATOR_SF_REWARD_INTERVAL.
* @return returns the pointer to the newly created TCompStateF node casted to (void *)
*/
void * allocateComparatorFormula(double val_bound_left, double val_bound_right, void * pSubForm, const int unary_type){
	PTCompStateF pCompStateF = (PTCompStateF) calloc(1, sizeof(TCompStateF));
	
	initializeTUnaryOp( &(pCompStateF->unary_op), COMPARATOR_SF, unary_type, pSubForm );
	
	pCompStateF->val_bound_left = val_bound_left;
	pCompStateF->val_bound_right = val_bound_right;
	
	//Set the pCompStateF field for the LONG_STEADY_F and UNTIL_PF subformulas
	PTFTypeRes pFTypeResSubForm = (PTFTypeRes) pSubForm;
	switch( pFTypeResSubForm->formula_type ){
		case LONG_STEADY_F:
			( (PTLongSteadyF) pSubForm )->pCompStateF = pCompStateF;
			break;
		case UNTIL_PF:
			( (PTUntilF) pSubForm )->pCompStateF = pCompStateF;
			break;
	}
	
	return (void *) pCompStateF;
}

/**
* Creates a steady-state/long-run formula node (LONG_STEADY_F): L, S
* Without the probability constraint.
* @param pSubForm the pointer for the subformula
* @param unary_type one of: LONG_STEADY_F_LONG_RUN, LONG_STEADY_F_STEADY_STATE
* @return returns the pointer to the newly created TLongSteadyF node casted to (void *)
*/
void * allocateLongSteadyFormula(void * pSubForm, const int unary_type){
	PTLongSteadyF pLongSteadyF = (PTLongSteadyF) calloc(1, sizeof(TLongSteadyF));
	
	//WARNING: This works until TLongSteadyF has TUnaryOp as the first field
	initializeTUnaryOp( (PTUnaryOp) pLongSteadyF, LONG_STEADY_F, unary_type, pSubForm );
	
	//NOTE: This field will be set when the outer PTCompStateF is allocated
	pLongSteadyF->pCompStateF = NULL;
	
	return (void *) pLongSteadyF;
}
/**
* Creates a pure reward-formula structure (PURE_REWARD_SF) such as C, E, Y operators of PRCTL
* For the reward bounds should be wrapped with the COMPARATOR_SF structure of unary_type:
*	COMPARATOR_SF_REWARD_INTERVAL.
* @param time the time instance
* @param pSubForm the subformula for the reward formula
* @param unary_type one of: PURE_REWARD_SF_EXPECTED_RR, PURE_REWARD_SF_INSTANT_R,
*					PURE_REWARD_SF_EXPECTED_AR
* @return returns the pointer to the newly created TPureRewardF node casted to (void *)
*/
void * allocatePureRewardFormula(double time, void * pSubForm, const int unary_type){
	PTPureRewardF pPureRewardF = (PTPureRewardF) calloc(1, sizeof(TPureRewardF));
	
	initializeTUnaryOp( &(pPureRewardF->unary_op), PURE_REWARD_SF, unary_type, pSubForm );
	
	pPureRewardF->time = time;
	
	return (void *) pPureRewardF;
}

/**
* Creates a next-formula structure (NEXT_PF) such as X operator of PCTL, CSL, PRCTL, CSRL
* @param left_time_bound the lower time bound
* @param right_time_bound the upper time bound
* @param left_reward_bound the left reward constraint
* @param right_reward_bound the right reward constraint
* @param pSubForm the subformula for the until formula
* @param unary_type one of: NEXT_PF_UNB, NEXT_PF_TIME, NEXT_PF_TIME_REWARD
* @return returns the pointer to the newly created TNextF node casted to (void *)
*/
void * allocateNextPathFormula(double left_time_bound, double right_time_bound,
				double left_reward_bound, double right_reward_bound,
				void * pSubForm, const int unary_type){
	PTNextF pNextF = (PTNextF) calloc(1, sizeof(TNextF));
	
	initializeTUnaryOp( &(pNextF->unary_op), NEXT_PF, unary_type, pSubForm );
	
	pNextF->left_time_bound = left_time_bound;
	pNextF->right_time_bound = right_time_bound;
	pNextF->left_reward_bound = left_reward_bound;
	pNextF->right_reward_bound = right_reward_bound;
	
	return (void *) pNextF;
}

/**
* Creates an until-formula structure (UNTIL_PF) such as U operator of PCTL, CSL, PRCTL, CSRL
* @param left_time_bound the lower time bound
* @param right_time_bound the upper time bound
* @param left_reward_bound the left reward constraint
* @param right_reward_bound the right reward constraint
* @param pSubFormL the left subformula for the until formula
* @param pSubFormR the right subformula for the until formula
* @param binary_type one of: UNTIL_PF_UNB, UNTIL_PF_TIME, UNTIL_PF_TIME_REWARD
* @return returns the pointer to the newly created TUntilF node casted to (void *)
*/
void * allocateUntilPathFormula(double left_time_bound, double right_time_bound,
				double left_reward_bound, double right_reward_bound,
				void * pSubFormL, void * pSubFormR, const int binary_type){
	PTUntilF pUntilF = (PTUntilF) calloc(1, sizeof(TUntilF));
	
	initializeTBinaryOp( &(pUntilF->binary_op), UNTIL_PF, binary_type, pSubFormL, pSubFormR );
	
	//NOTE: This field will be set when the outer PTCompStateF is allocated
	pUntilF->pCompStateF = NULL;
	
	pUntilF->left_time_bound = left_time_bound;
	pUntilF->right_time_bound = right_time_bound;
	pUntilF->left_reward_bound = left_reward_bound;
	pUntilF->right_reward_bound = right_reward_bound;
	
	return (void *) pUntilF;
}

/**
* This function is used to traverse throught the formula tree and call
* the parameter function for the tree nodes.
* @param before if TRUE then the functions are called before the recursive call
* @param after if TRUE then the functions are called after the recursive call
* @param between if TRUE then the functions are called between the recursive calls
*			this is valid for the UNTIL_PF and BINARY_OP_SF operators only
*			and used, e.g., in printing the formula tree.
* @param pFormulaTreeNode the formula tree node to begin with
* @param pFPTFTypeResBefore the pointer to a function of type TPFunctOnPTFTypeRes
*					if not NULL then allways called before the recursive call
* @param pFPTFTypeResAfter the pointer to a function of type TPFunctOnPTFTypeRes
*					if not NULL then allways called after the recursive call
* @param pFPTAtomicF the pointer to a function of type TPFunctOnPTAtomicF
* @param pFPTUnaryOp the pointer to a function of type TPFunctOnPTUnaryOp
* @param pFPTLongSteadyF the pointer to a function of type TPFunctOnPTLongSteadyF
* @param pFPTBinaryOp the pointer to a function of type TPFunctOnPTBinaryOp
* @param pFPTCompStateF the pointer to a function of type TPFunctOnPTCompStateF
* @param pFPTPureRewardF the pointer to a function of type TPFunctOnPTPureRewardF
* @param pFPTNextF the pointer to a function of type TPFunctOnPTNextF
* @param pFPTUntilF the pointer to a function of type TPFunctOnPTUntilF
*/
void doFormulaTreeTraversal( void * pFormulaTreeNode, const BOOL before, const BOOL after,
				const BOOL between, TPFunctOnPTFTypeRes pFPTFTypeResBefore,
				TPFunctOnPTFTypeRes pFPTFTypeResAfter, TPFunctOnPTAtomicF pFPTAtomicF,
				TPFunctOnPTUnaryOp pFPTUnaryOp, TPFunctOnPTLongSteadyF pFPTLongSteadyF,
				TPFunctOnPTBinaryOp pFPTBinaryOp, TPFunctOnPTCompStateF pFPTCompStateF,
				TPFunctOnPTPureRewardF pFPTPureRewardF, TPFunctOnPTNextF pFPTNextF,
				TPFunctOnPTUntilF pFPTUntilF){
	PTFTypeRes pFTypeRes = (PTFTypeRes) pFormulaTreeNode;
	BOOL call_rec = TRUE;
	
	//Define all possible node-type variables
	PTAtomicF pAtomicF; PTUnaryOp pUnaryOp; PTLongSteadyF pLongSteadyF; PTBinaryOp pBinaryOp;
	PTCompStateF pCompStateF; PTPureRewardF pPureRewardF; PTNextF pNextF; PTUntilF pUntilF;
	
	if( pFTypeRes != NULL ){
		if( pFPTFTypeResBefore != NULL ){
			pFPTFTypeResBefore( TRUE, pFTypeRes );
		}
		switch( pFTypeRes->formula_type ){
			case ATOMIC_SF:
				pAtomicF = (PTAtomicF) pFormulaTreeNode;
				pFPTAtomicF( TRUE, pAtomicF );
				break;
			case UNARY_OP_SF:
				pUnaryOp = (PTUnaryOp) pFormulaTreeNode;
				if( before ){
					call_rec = pFPTUnaryOp( TRUE, pUnaryOp );
				}
				if( call_rec ) {
					doFormulaTreeTraversal(pUnaryOp->pSubForm, before, after, between,
								pFPTFTypeResBefore,
								pFPTFTypeResAfter, pFPTAtomicF, pFPTUnaryOp,
								pFPTLongSteadyF, pFPTBinaryOp, pFPTCompStateF,
								pFPTPureRewardF, pFPTNextF, pFPTUntilF);
				}
				if( after ){
					pFPTUnaryOp( FALSE, pUnaryOp );
				}
				break;
			case LONG_STEADY_F:
				pLongSteadyF = (PTLongSteadyF) pFormulaTreeNode;
				if( before ){
					call_rec = pFPTLongSteadyF( TRUE, pLongSteadyF );
				}
				if( call_rec ) {
					doFormulaTreeTraversal(pLongSteadyF->unary_op.pSubForm, before, after, between,
								pFPTFTypeResBefore,
								pFPTFTypeResAfter, pFPTAtomicF, pFPTUnaryOp,
								pFPTLongSteadyF, pFPTBinaryOp, pFPTCompStateF,
								pFPTPureRewardF, pFPTNextF, pFPTUntilF);
				}
				if( after ){
					pFPTLongSteadyF( FALSE, pLongSteadyF );
				}
				break;
			case BINARY_OP_SF:
				pBinaryOp = (PTBinaryOp) pFormulaTreeNode;
				if( before ){
					call_rec = pFPTBinaryOp( TRUE, FALSE, pBinaryOp );
				}
				if( call_rec ) {
					doFormulaTreeTraversal(pBinaryOp->pSubFormL, before, after, between,
								pFPTFTypeResBefore,
								pFPTFTypeResAfter, pFPTAtomicF, pFPTUnaryOp,
								pFPTLongSteadyF, pFPTBinaryOp, pFPTCompStateF,
								pFPTPureRewardF, pFPTNextF, pFPTUntilF);
					if( between ){
						pFPTBinaryOp( FALSE, TRUE, pBinaryOp );
					}
					doFormulaTreeTraversal(pBinaryOp->pSubFormR, before, after, between,
								pFPTFTypeResBefore,
								pFPTFTypeResAfter, pFPTAtomicF, pFPTUnaryOp,
								pFPTLongSteadyF, pFPTBinaryOp, pFPTCompStateF,
								pFPTPureRewardF, pFPTNextF, pFPTUntilF);
				}
				if( after ){
					pFPTBinaryOp( FALSE, FALSE, pBinaryOp );
				}
				break;
			case COMPARATOR_SF:
				pCompStateF = (PTCompStateF) pFormulaTreeNode;
				if( before ){
					call_rec = pFPTCompStateF( TRUE, pCompStateF );
				}
				if( call_rec ) {
					doFormulaTreeTraversal(pCompStateF->unary_op.pSubForm, before, after, between,
								pFPTFTypeResBefore,
								pFPTFTypeResAfter, pFPTAtomicF, pFPTUnaryOp,
								pFPTLongSteadyF, pFPTBinaryOp, pFPTCompStateF,
								pFPTPureRewardF, pFPTNextF, pFPTUntilF);
				}
				if( after ){
					pFPTCompStateF( FALSE, pCompStateF );
				}
				break;
			case PURE_REWARD_SF:
				pPureRewardF = (PTPureRewardF) pFormulaTreeNode;
				if( before ){
					call_rec = pFPTPureRewardF( TRUE, pPureRewardF );
				}
				if( call_rec ) {
					doFormulaTreeTraversal(pPureRewardF->unary_op.pSubForm, before, after, between,
								pFPTFTypeResBefore, pFPTFTypeResAfter,
								pFPTAtomicF, pFPTUnaryOp, pFPTLongSteadyF, pFPTBinaryOp,
								pFPTCompStateF, pFPTPureRewardF, pFPTNextF, pFPTUntilF);
				}
				if( after ){
					pFPTPureRewardF( FALSE, pPureRewardF );
				}
				break;
			case NEXT_PF:
				pNextF = (PTNextF) pFormulaTreeNode;
				
				if( before ){
					call_rec = pFPTNextF( TRUE, pNextF );
				}
				if( call_rec ) {
					doFormulaTreeTraversal(pNextF->unary_op.pSubForm, before, after, between,
								pFPTFTypeResBefore,
								pFPTFTypeResAfter, pFPTAtomicF, pFPTUnaryOp,
								pFPTLongSteadyF, pFPTBinaryOp, pFPTCompStateF,
								pFPTPureRewardF, pFPTNextF, pFPTUntilF);
				}
				if( after ){
					pFPTNextF( FALSE, pNextF );
				}
				break;
			case UNTIL_PF:
				pUntilF = (PTUntilF) pFormulaTreeNode;
				if( before ){
					call_rec = pFPTUntilF( TRUE, FALSE, pUntilF );
				}
				if( call_rec ) {
					doFormulaTreeTraversal(pUntilF->binary_op.pSubFormL, before, after, between,
								pFPTFTypeResBefore,
								pFPTFTypeResAfter, pFPTAtomicF, pFPTUnaryOp, pFPTLongSteadyF,
								pFPTBinaryOp, pFPTCompStateF, pFPTPureRewardF, pFPTNextF,
								pFPTUntilF);
					if( between ){
						pFPTUntilF( FALSE, TRUE, pUntilF );
					}
					doFormulaTreeTraversal(pUntilF->binary_op.pSubFormR, before, after, between,
								pFPTFTypeResBefore,
								pFPTFTypeResAfter, pFPTAtomicF, pFPTUnaryOp, pFPTLongSteadyF,
								pFPTBinaryOp, pFPTCompStateF, pFPTPureRewardF, pFPTNextF,
								pFPTUntilF);
				}
				if( after ){
					pFPTUntilF( FALSE, FALSE, pUntilF );
				}
				break;
			default:
				printf("ERROR: An unknown formula node type %d.", pFTypeRes->formula_type);
				exit(1);
		}
		if( pFPTFTypeResAfter != NULL ){
			pFPTFTypeResAfter( FALSE, pFTypeRes );
		}
	}
}

/***********************************************************************/
/****METHODS, NEEDED TO FREE THE FORMULA TREE USING TRAVERSAL METHOD****/
/*************************doFormulaTreeTraversal************************/
/***********************************************************************/

static BOOL freePTFTypeRes( BOOL before, PTFTypeRes pFTypeRes ){
	if( pFTypeRes->pCharFormulaString != NULL ){
		free( pFTypeRes->pCharFormulaString );
	}
	
	if( pFTypeRes->pYesBitsetResult != NULL ){
		free_bitset( pFTypeRes->pYesBitsetResult );
	}
	if( pFTypeRes->pNoBitsetResult != NULL ){
		free_bitset( pFTypeRes->pNoBitsetResult );
	}
	if( pFTypeRes->pProbCILeftBorder != NULL ){
		free( pFTypeRes->pProbCILeftBorder );
	}
	if( pFTypeRes->pProbCIRightBorder != NULL ){
		free( pFTypeRes->pProbCIRightBorder );
	}
	
	if( pFTypeRes->pProbRewardResult != NULL ){
		free( pFTypeRes->pProbRewardResult );
	}
	if( pFTypeRes->pErrorBound != NULL ){
		free( pFTypeRes->pErrorBound );
	}
	return FALSE;
}

static BOOL freePTAtomicF( BOOL before, PTAtomicF pAtomicF){
	free(pAtomicF->pName);
	free(pAtomicF);
	return FALSE;
}

static BOOL freePTUnaryOp( BOOL before, PTUnaryOp pUnaryOp ){
	free(pUnaryOp);
	return FALSE;
}

static BOOL freePTLongSteadyF( BOOL before, PTLongSteadyF pLongSteadyF ){
	free(pLongSteadyF);
	return FALSE;
}

static BOOL freePTBinaryOp( BOOL before, BOOL between, PTBinaryOp pBinaryOp ){
	free(pBinaryOp);
	return FALSE;
}

static BOOL freePTCompStateF( BOOL before, PTCompStateF pCompStateF ){
	free(pCompStateF);
	return FALSE;
}

static BOOL freePTPureRewardF( BOOL before, PTPureRewardF pPureRewardF ){
	free(pPureRewardF);
	return FALSE;
}

static BOOL freePTNextF( BOOL before, PTNextF pNextF ){
	free(pNextF);
	return FALSE;
}

static BOOL freePTUntilF( BOOL before, BOOL between, PTUntilF pUntilF ){
	free(pUntilF);
	return FALSE;
}

/**
* This method deallocates memory allocated for the tree which has the given root node.
* @param pFormulaTreeNode the root tree node scheduled for deallocation
*/
void freeFormulaTree(void * pFormulaTreeNode){
	doFormulaTreeTraversal( pFormulaTreeNode, FALSE, TRUE, FALSE, freePTFTypeRes, NULL, freePTAtomicF, freePTUnaryOp,
				freePTLongSteadyF, freePTBinaryOp, freePTCompStateF, freePTPureRewardF,
				freePTNextF, freePTUntilF);
}

/***********************************************************************/
/*****METHODS NEEDED TO DUMP THE FORMULA TREE USING TRAVERSAL METHOD****/
/*************************doFormulaTreeTraversal************************/
/***********************************************************************/

/**
* This function appends the templated string to the given string
* @param ppToString the pointer to a pointer to astring we want to print to.
* @param fmt the template string
* @param args the template argiments
*/
static void appendToString(char** ppToString, const char *fmt, ...){
	char *pTmpString = NULL, *pNewString = NULL;
	va_list args;
	va_start(args, fmt);
	
	if( *ppToString != NULL ){
		asprintf( &pTmpString, "%s%s", *ppToString, fmt );
		vasprintf( &pNewString, pTmpString, args );
		free( pTmpString );
		free( *ppToString );
	} else {
		vasprintf( &pNewString, fmt, args );
	}
	
	*ppToString = pNewString;
}

static BOOL dumpPTFTypeRes( BOOL before, PTFTypeRes pFTypeRes ){
	
	return FALSE; //The result actually should not matter at all
}

/**
* This is a dedicated function for printing the node results and main parameters
* @param pFTypeRes the node to print
*/
void printFormulaBitsetResultParams(PTFTypeRes pFTypeRes){
	IF_SAFETY( pFTypeRes != NULL)
		if( pFTypeRes->pCharFormulaString != NULL ){
			printf("%s: %s\n", FORMULA_STR, pFTypeRes->pCharFormulaString);
		}
		PTCompStateF pCompStateF = (PTCompStateF) pFTypeRes;
		if( pFTypeRes->formula_type == COMPARATOR_SF ){
			PTFTypeRes pFTypeResSubForm = (PTFTypeRes) pCompStateF->unary_op.pSubForm;
			if( isSimulationOn() || pFTypeRes->doSimHere || pFTypeRes->doSimBelow ){
				printf("%s: %s\n", SIMULATED_STR, ( pFTypeResSubForm->doSimHere ? YES_STR : NO_STR ) );
				if( pFTypeRes->isSimOneInitState ){
					printf("%s: %d\n", INITIAL_STATE_STR, pFTypeRes->initial_state + 1);
				}
			}
			if( pFTypeResSubForm->doSimHere ){
				printf("%s: %e\n", CONFIDENCE_STR, pFTypeResSubForm->confidence );
				print_state_probs( pFTypeResSubForm->prob_result_size,
						pFTypeResSubForm->pProbCILeftBorder, LEFT_CI_RESULT_STR );
				print_state_probs( pFTypeResSubForm->prob_result_size,
						pFTypeResSubForm->pProbCIRightBorder, RIGHT_CI_RESULT_STR );
			} else {
				printf("%s: %e\n", ERROR_BOUND_STR, pFTypeResSubForm->error_bound );
				print_state_probs( pFTypeResSubForm->prob_result_size,
						pFTypeResSubForm->pProbRewardResult, LEFT_RIGHT_RESULT_STR );
			}
		}
		if( pFTypeRes->doSimHere || pFTypeRes->doSimBelow ){
			//We do not distinguish between one- and all- initial state simulations here
			//because in case of the one initial state simulation the bitsts may contain
			//at most the provided initial state, see: modelCheckUnboundedUntilCTMC
			//in the simulation_ctmc.c file
			print_states( pFTypeRes->pYesBitsetResult, YES_STATES_STR );
			print_states( pFTypeRes->pNoBitsetResult, NO_STATES_STR );
		} else {
			print_states( pFTypeRes->pYesBitsetResult, YES_NO_STATES_STR );
		}
		printf("\n");
	ELSE_SAFETY
		printf("ERROR: Trying to print a NULL formula-tree node.\n");
		exit(1);
	ENDIF_SAFETY
}

static BOOL dumpPTAtomicF( BOOL before, PTAtomicF pAtomicF){
	PTFTypeRes pFTypeRes = (PTFTypeRes) pAtomicF;
	
	//The tree might have been printed before and then the 
	//Value of pFTypeRes->pCharFormulaString has been set.
	if( pFTypeRes->pCharFormulaString == NULL ){
		//We do not care about 'before' because it is called only once,
		//since there is no recursion for subformulas
		appendToString( &pFTypeRes->pCharFormulaString, "%s", pAtomicF->pName);
	}
		
	//Print the node results and parameters
	printFormulaBitsetResultParams( pFTypeRes );
	return FALSE; //The result actually should not matter at all
}

static BOOL dumpPTUnaryOp( BOOL before, PTUnaryOp pUnaryOp ){
	PTFTypeRes pFTypeRes, pFTypeResSubForm;
	if( ! before ){
		pFTypeRes = (PTFTypeRes) pUnaryOp;
		
		//The tree might have been printed before and then the 
		//Value of pFTypeRes->pCharFormulaString has been set.
		if( pFTypeRes->pCharFormulaString == NULL ){
			pFTypeResSubForm = (PTFTypeRes) pUnaryOp->pSubForm;
			switch( pUnaryOp->unary_type ){
				case UNARY_OP_SF_NEGATION:
					appendToString( &pFTypeRes->pCharFormulaString, "!%s", pFTypeResSubForm->pCharFormulaString );
					break;
				case UNARY_OP_SF_BRACES:
					appendToString( &pFTypeRes->pCharFormulaString, "( %s )", pFTypeResSubForm->pCharFormulaString );
					break;
				default :
					printf("\nERROR: An unexpected subtype '%d' of UNARY_OP_SF.\n", pUnaryOp->unary_type);
					exit(1);
			}
		}
		
		//NOTE: In case of braced formula (A) we do not
		//print the result for it but only for its subformula A.
		if( pUnaryOp->unary_type == UNARY_OP_SF_NEGATION){
			//Print the node results and parameters
			printFormulaBitsetResultParams( pFTypeRes );
		}
	}
	return TRUE;
}

//Propogates the pCharFormulaString from the subformula for easier access and printing
static inline void propogateFormulaStringFromSubForm(PTFTypeRes pFTypeRes, PTFTypeRes pFTypeResSubForm){
	appendToString( &pFTypeRes->pCharFormulaString, "%s", pFTypeResSubForm->pCharFormulaString );
}

static BOOL dumpPTLongSteadyF( BOOL before, PTLongSteadyF pLongSteadyF ){
	PTFTypeRes pFTypeRes;
	if(! before ){
		pFTypeRes = (PTFTypeRes) pLongSteadyF;
		
		//The tree might have been printed before and then the 
		//Value of pFTypeRes->pCharFormulaString has been set.
		if( pFTypeRes->pCharFormulaString == NULL ){
			propogateFormulaStringFromSubForm( pFTypeRes, (PTFTypeRes) pLongSteadyF->unary_op.pSubForm);
		}
	}
	//WARNING: Do not print anything because it is all printer
	//uniformly from the dumpPTCompStateF method
	return TRUE;
}

static BOOL dumpPTBinaryOp( BOOL before, BOOL between, PTBinaryOp pBinaryOp ){
	PTFTypeRes pFTypeRes, pFTypeResSubFormL, pFTypeResSubFormR;
	char * pCharOp;
	if( ! before && ! between ){
		pFTypeRes = (PTFTypeRes) pBinaryOp;
		
		//The tree might have been printed before and then the 
		//Value of pFTypeRes->pCharFormulaString has been set.
		if( pFTypeRes->pCharFormulaString == NULL ){
			pFTypeResSubFormL = (PTFTypeRes) pBinaryOp->pSubFormL;
			pFTypeResSubFormR = (PTFTypeRes) pBinaryOp->pSubFormR;
			switch( pBinaryOp->binary_type ){
				case BINARY_OP_SF_OR:
					pCharOp = " || ";
					break;
				case BINARY_OP_SF_AND:
					pCharOp = " && ";
					break;
				case BINARY_OP_SF_IMPLIES:
					pCharOp = " => ";
					break;
				default :
					printf("\nERROR: An unexpected subtype '%d' of BINARY_OP_SF.\n", pBinaryOp->binary_type);
					exit(1);
			}
			appendToString( &pFTypeRes->pCharFormulaString, "%s%s%s", pFTypeResSubFormL->pCharFormulaString,
					pCharOp, pFTypeResSubFormR->pCharFormulaString );
		}
		
		//Print the node results and parameters
		printFormulaBitsetResultParams( pFTypeRes );
	}
	return TRUE;
}

static void printComparatorByType( PTCompStateF pCompStateF, char ** ppCharFormulaString ){
	const int comp_type = pCompStateF->unary_op.unary_type;
	if( comp_type == COMPARATOR_SF_REWARD_INTERVAL ){
		appendToString( ppCharFormulaString, "[%e, %e] ", pCompStateF->val_bound_left, pCompStateF->val_bound_right);
	} else {
		appendToString( ppCharFormulaString, "{ ");
		switch( comp_type ){
			case COMPARATOR_SF_LESS:
				appendToString( ppCharFormulaString, "<");
				break;
			case COMPARATOR_SF_LESS_OR_EQUAL:
				appendToString( ppCharFormulaString, "<=");
				break;
			case COMPARATOR_SF_GREATER:
				appendToString( ppCharFormulaString, ">");
				break;
			case COMPARATOR_SF_GREATER_OR_EQUAL:
				appendToString( ppCharFormulaString, ">=");
				break;
			default:
				printf("\nERROR: An unexpected subtype '%d' of COMPARATOR_SF.\n", pCompStateF->unary_op.unary_type );
				exit(1);
		}
		appendToString( ppCharFormulaString, " %e }", pCompStateF->val_bound_left);
	}
}

static BOOL dumpPTCompStateF( BOOL before, PTCompStateF pCompStateF ){
	if( ! before ){
		PTFTypeRes pFTypeRes = (PTFTypeRes) pCompStateF;
		//The tree might have been printed before and then the 
		//Value of pFTypeRes->pCharFormulaString has been set.
		if( pFTypeRes->pCharFormulaString == NULL ){
			char *pCharSubFormString = NULL;
			PTLongSteadyF pLongSteadyF = NULL; PTPureRewardF pPureRewardF = NULL;
			PTFTypeRes pFTypeResSubForm = (PTFTypeRes) pCompStateF->unary_op.pSubForm;
			switch( pFTypeResSubForm->formula_type ){
				case LONG_STEADY_F:
					pLongSteadyF = (PTLongSteadyF) pFTypeResSubForm;
					switch( pLongSteadyF->unary_op.unary_type ){
						case LONG_STEADY_F_LONG_RUN:
							appendToString( &pFTypeRes->pCharFormulaString, "L");
							break;
						case LONG_STEADY_F_STEADY_STATE:
							appendToString( &pFTypeRes->pCharFormulaString, "S");
							break;
						default :
							printf("\nERROR: An unexpected subtype '%d' of LONG_STEADY_F.\n", pLongSteadyF->unary_op.unary_type );
							exit(1);
					}
					break;
				case PURE_REWARD_SF:
					pPureRewardF = (PTPureRewardF) pFTypeResSubForm;
					switch( pPureRewardF->unary_op.unary_type ){
						case PURE_REWARD_SF_EXPECTED_RR:
							appendToString( &pFTypeRes->pCharFormulaString, "E");
							break;
						case PURE_REWARD_SF_INSTANT_R:
							appendToString( &pFTypeRes->pCharFormulaString, "C");
							break;
						case PURE_REWARD_SF_EXPECTED_AR:
							appendToString( &pFTypeRes->pCharFormulaString, "Y");
							break;
						default :
							printf("\nERROR: An unexpected subtype '%d' of LONG_STEADY_F.\n", pPureRewardF->unary_op.unary_type );
							exit(1);
					}
					if( pPureRewardF->time > 0){
						appendToString( &pFTypeRes->pCharFormulaString, "[%e]", pPureRewardF->time );
					}
					break;
				case NEXT_PF:
				case UNTIL_PF:
					appendToString( &pFTypeRes->pCharFormulaString, "P");
					break;
				default :
					printf("\nERROR: An unexpected type '%d' of COMPARATOR_SF subformula.\n", pFTypeResSubForm->formula_type);
					exit(1);
			}
			printComparatorByType( pCompStateF, &pFTypeRes->pCharFormulaString );
			pCharSubFormString = pFTypeResSubForm->pCharFormulaString;
			appendToString( &pFTypeRes->pCharFormulaString, "[ %s ]", pCharSubFormString);
		}
		
		//Print the node results and parameters
		printFormulaBitsetResultParams( pFTypeRes );
	}
	return TRUE;
}

static BOOL dumpPTPureRewardF( BOOL before, PTPureRewardF pPureRewardF ){
	PTFTypeRes pFTypeRes;
	if(! before ){
		pFTypeRes = (PTFTypeRes) pPureRewardF;
		
		//The tree might have been printed before and then the 
		//Value of pFTypeRes->pCharFormulaString has been set.
		if( pFTypeRes->pCharFormulaString == NULL ){
			propogateFormulaStringFromSubForm( pFTypeRes, (PTFTypeRes) pPureRewardF->unary_op.pSubForm);
		}
	}
	//WARNING: Do not print anything because it is all printer
	//uniformly from the dumpPTCompStateF method
	return TRUE;
}

static BOOL dumpPTNextF( BOOL before, PTNextF pNextF ){
	PTFTypeRes pFTypeRes, pFTypeResSubForm;
	if( ! before ){
		pFTypeRes = (PTFTypeRes) pNextF;
		
		//The tree might have been printed before and then the 
		//Value of pFTypeRes->pCharFormulaString has been set.
		if( pFTypeRes->pCharFormulaString == NULL ){
			pFTypeResSubForm = (PTFTypeRes) pNextF->unary_op.pSubForm;
			switch( pNextF->unary_op.unary_type ){
				case NEXT_PF_UNB:
					appendToString( &pFTypeRes->pCharFormulaString, "X %s", pFTypeResSubForm->pCharFormulaString);
					break;
				case NEXT_PF_TIME:
					appendToString( &pFTypeRes->pCharFormulaString, "X[%e, %e] %s", pNextF->left_time_bound,
						pNextF->right_time_bound, pFTypeResSubForm->pCharFormulaString );
					break;
				case NEXT_PF_TIME_REWARD:
					appendToString( &pFTypeRes->pCharFormulaString, "X[%e, %e][%e, %e] %s", pNextF->left_time_bound,
						pNextF->right_time_bound, pNextF->left_reward_bound, pNextF->right_reward_bound,
						pFTypeResSubForm->pCharFormulaString );
					break;
				default:
					printf("\nERROR: An unexpected subtype '%d' of NEXT_PF.\n", pNextF->unary_op.unary_type);
					exit(1);
			}
		}
	}
	//WARNING: Do not print anything because it is all printer
	//uniformly from the dumpPTCompStateF method
	return TRUE;
}

static BOOL dumpPTUntilF( BOOL before, BOOL between, PTUntilF pUntilF ){
	PTFTypeRes pFTypeRes, pFTypeResSubFormL, pFTypeResSubFormR;
	if( ! before && ! between ){
		pFTypeRes = (PTFTypeRes) pUntilF;
		
		//The tree might have been printed before and then the 
		//Value of pFTypeRes->pCharFormulaString has been set.
		if( pFTypeRes->pCharFormulaString == NULL ){
			pFTypeResSubFormL = (PTFTypeRes) pUntilF->binary_op.pSubFormL;
			pFTypeResSubFormR = (PTFTypeRes) pUntilF->binary_op.pSubFormR;
			switch( pUntilF->binary_op.binary_type ){
				case UNTIL_PF_UNB:
					appendToString( &pFTypeRes->pCharFormulaString, "%s U %s", pFTypeResSubFormL->pCharFormulaString,
						pFTypeResSubFormR->pCharFormulaString );
					break;
				case UNTIL_PF_TIME:
					appendToString( &pFTypeRes->pCharFormulaString, "%s U[%e, %e] %s", pFTypeResSubFormL->pCharFormulaString,
						pUntilF->left_time_bound, pUntilF->right_time_bound, pFTypeResSubFormR->pCharFormulaString );
					break;
				case UNTIL_PF_TIME_REWARD:
					appendToString( &pFTypeRes->pCharFormulaString, "%s U[%e, %e][%e, %e] %s",
						pFTypeResSubFormL->pCharFormulaString, pUntilF->left_time_bound,
						pUntilF->right_time_bound, pUntilF->left_reward_bound,
						pUntilF->right_reward_bound, pFTypeResSubFormR->pCharFormulaString );
					break;
				default:
					printf("\nERROR: An unexpected subtype '%d' of UNTIL_PF.\n", pUntilF->binary_op.binary_type);
					exit(1);
			}
		}
	}
	//WARNING: Do not print anything because it is all printer
	//uniformly from the dumpPTCompStateF method
	return TRUE;
}

/**
* This method prints for the tree which has the given root node.
* @param pFormulaTreeNode the root tree node scheduled for printing
*/
void dumpFormulaTree(void * pFormulaTreeNode){
	doFormulaTreeTraversal( pFormulaTreeNode, FALSE, TRUE, FALSE, dumpPTFTypeRes, NULL, dumpPTAtomicF, dumpPTUnaryOp,
				dumpPTLongSteadyF, dumpPTBinaryOp, dumpPTCompStateF, dumpPTPureRewardF,
				dumpPTNextF, dumpPTUntilF);
	//Printing the new line symbl after the formula has been printed.
}
