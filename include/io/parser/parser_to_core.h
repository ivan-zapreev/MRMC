/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: parser_to_core.h,v $
*	$Revision: 1.4 $
*	$Date: 2007/10/05 10:56:23 $
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
*	the parser and the core model checkier.
*/

#include "help.h"

#ifndef PARSET_TO_CORE
#define PARSER_TO_CORE

/**
* This method invokes recursive model-checking procedure of the parsed formula
* @param pResultBitset the pointer to the formula tree, that has to be model checked
*/
inline void modelCheckFormula( void* pResultBitset );

/**
* This procedure does printing of the error messages stored in the help.h file.
* @param help_msg_type the help message type, one of:
*	HELP_GENERAL_MSG_TYPE, HELP_COMMON_MSG_TYPE, HELP_REWARDS_MSG_TYPE,
*	HELP_SIMULATION_MSG_TYPE, HELP_LOGIC_MSG_TYPE.
*/
inline void printHelpMessage(const int help_msg_type);

/**
* Prints the formula tree
*/
void printResultFormulaTree();

/**
* This method is used for printing state probability if any.
* In case get_result_probs_size() == 0, get_result_probs() == NULL
* or the index is out of bounds, the error messages are printed.
* @param index: the state index >= 1
*/

inline void printResultingStateProbability( const int index);

/**
* This method is used for printing state satisfyability result if any.
* I.e. if the state with the given index satisfyes the previously checked
* stateformula. In case get_result_bitset() == NULL or the index is out
* of bounds, the error messages are printed.
* @param index: the state index >= 1
*/
inline printResultingStateSatisfyability( const int index );

/**
* This procedure sets the results of the stateformula into the runtime.c
* using the set_result_bitset(void *).
* This method is also used for printing the resulting probabilities if
* any and the set of states that satisfy the formula.
* NOTE: Printing is done only in case when the call of isPrintingOn()
* returns TRUE, get_result_probs() != NULL or get_result_bitset() != NULL
*/
inline void printFormulaResults();

/**
* Start the timer.
* NOTE: Here the start of the timer should be done only in case we have an atomic
* proposition, 'tt' or 'ff' formula, becuase model checking starts from the leafs.
* WARNING: BE CAREFULL USING THIS MEHTOD. THERE SHOULD BE NO CONCURRENT USE OF
* THE TIMER FROM DIFFERENT ALGORITHMS AND SUCH.
* For example if we use this method for monitoring elapsed time of model checking and
* formula independent lumping, which do not overlap in time!
*/
inline void startTimer();

/**
* Stop the timer and return the elapsed time in milliseconds.
* NOTE: Here the stop of the timer should be done only in case the top level
* state formula procedure is called. I.e. stateformula NEWLINE.
* WARNING: BE CAREFULL USING THIS MEHTOD. THERE SHOULD BE NO CONCURRENT USE OF
* THE TIMER FROM DIFFERENT ALGORITHMS AND SUCH.
* For example if we use this method for monitoring elapsed time of model checking and
* formula independent lumping, which do not overlap in time!
* @return elapsed time in milliseconds
*/
inline long stopTimer();

/**
* This method is responsible for cleaning the old model-checking formula tree.
*/
inline void clearOldModelCheckingResults();

/**
* This method does bottom-up traversal of the formula tree
* along with model checking the formulas. It invokes printing
* the results if neede and is also responsiblce for outputting
* the model-checking time.
* @param pFormulaTreeRootNode the formula tree root node
*/
inline void modelCheckFormulaTree( void * pFormulaTreeRootNode );

#endif
