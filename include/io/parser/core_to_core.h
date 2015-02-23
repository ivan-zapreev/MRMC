/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: core_to_core.h,v $
*	$Revision: 1.5 $
*	$Date: 2007/10/04 11:18:30 $
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

#include "bitset.h"

#ifndef CORE_TO_CORE
#define CORE_TO_CORE
	
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
	inline BOOL modelCheckAtomicFormula( BOOL before, PTAtomicF pAtomicF );
	
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
	*/
	inline BOOL modelCheckBinaryOperator( BOOL before, BOOL between, PTBinaryOp pBinaryOp );
	
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
	inline BOOL modelCheckUnaryOperator( BOOL before, PTUnaryOp pUnaryOp );
	
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
	inline BOOL modelCheckComparatorFormula( BOOL before, PTCompStateF pCompStateF );
	
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
	inline BOOL modelCheckLongSteadyFormula( BOOL before, PTLongSteadyF pLongSteadyF );
	
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
	inline BOOL modelCheckPureRewardFormula( BOOL before, PTPureRewardF pPureRewardF );
	
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
	inline BOOL modelCheckNextFormula( BOOL before, PTNextF pNextF );
	
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
	inline BOOL modelCheckUntilFormula( BOOL before, BOOL between, PTUntilF pUntilF );
#endif
