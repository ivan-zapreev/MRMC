/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: parser_to_tree.h,v $
*	$Revision: 1.16 $
*	$Date: 2007/10/14 13:51:11 $
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

#include "bitset.h"

#ifndef PARSET_TO_TREE
#define PARSER_TO_TREE
	
	/*Define the result names for printing*/
	#define FORMULA_STR "$FORMULA"
	#define SIMULATED_STR "$SIMULATED"
	#define YES_STR "YES"
	#define NO_STR "NO"
	#define INITIAL_STATE_STR "$INITIAL STATE"
	#define CONFIDENCE_STR "$CONFIDENCE"
	#define ERROR_BOUND_STR "$ERROR BOUND"
	//The numerical results
	#define LEFT_RIGHT_RESULT_STR "$RESULT"
	#define LEFT_CI_RESULT_STR "$CI_LEFT_RESULT"
	#define RIGHT_CI_RESULT_STR "$CI_RIGHT_RESULT"
	//The logical results
	#define YES_NO_STATES_STR "$STATE"
	#define YES_STATES_STR "$YES_STATE"
	#define NO_STATES_STR "$NO_STATE"
	
	//Define strings for the BOOL constant
	#define NAME_FALSE "ff"
	#define NAME_TRUE "tt"
	
	/*The types of formulas we have in all the logics CSL, PCTL, CSRL, PRCTL*/
	//The atomic formula
	#define ATOMIC_SF 1
		/*The sub-types for the ATOMIC_SF type*/
		#define ATOMIC_SF_AP 1
		#define ATOMIC_SF_TT 2
		#define ATOMIC_SF_FF 3
	
	//The unary-operator formula, like negation or braces
	//The binary-operator formula, like &&, ||, =>
	#define UNARY_OP_SF 2
		/*The sub-types for the UNARY_OP_SF type*/
		#define UNARY_OP_SF_NEGATION 1
		#define UNARY_OP_SF_BRACES 2
	
	//The binary-operator formula, like &&, ||, =>
	#define BINARY_OP_SF 3
		/*The sub-types for the BINARY_OP_SF type*/
		#define BINARY_OP_SF_IMPLIES 1
		#define BINARY_OP_SF_AND 2
		#define BINARY_OP_SF_OR 3
	
	//The comparator formulas: >, >=, <, <=.
	//These are wrappers for the L, S, P operator probability results.
	#define COMPARATOR_SF 4
		/*The sub-types for the COMPARATOR_SF type*/
		#define COMPARATOR_SF_LESS 1
		#define COMPARATOR_SF_LESS_OR_EQUAL 2
		#define COMPARATOR_SF_GREATER 3
		#define COMPARATOR_SF_GREATER_OR_EQUAL 4
		#define COMPARATOR_SF_REWARD_INTERVAL 5
	
	//The long-run/seady-state formula, such as: S, L, without
	//the probability constraint part.
	#define LONG_STEADY_F 5
		/*The sub-types for the LONG_STEADY_F type*/
		#define LONG_STEADY_F_LONG_RUN 1
		#define LONG_STEADY_F_STEADY_STATE 2
	
	//The pure reward-oriented formula is either a E, C, or Y operators
	#define PURE_REWARD_SF 6
		/*The sub-types for the PURE_REWARD_SF type*/
		#define PURE_REWARD_SF_EXPECTED_RR 1
		#define PURE_REWARD_SF_INSTANT_R 2
		#define PURE_REWARD_SF_EXPECTED_AR 3
	
	//The next formula X
	#define NEXT_PF 7
		/*The sub-types for the NEXT_SF type*/
		#define NEXT_PF_UNB 1
		#define NEXT_PF_TIME 2
		#define NEXT_PF_TIME_REWARD 3
		
	//The next formula U
	#define UNTIL_PF 8
		/*The sub-types for the UNTIL_SF type*/
		#define UNTIL_PF_UNB 1
		#define UNTIL_PF_TIME 2
		#define UNTIL_PF_TIME_REWARD 3
		
	/**
	* All state formulas have their type and a resulting bitset and probabilities/rewards
	* formula_type - the formula type
	* pCharFormulaString - this field is filled if and when the formula tree is printed.
	*				it contains the foormula string of the entire sub-tree
	* prob_result_size - the size of pProbRewardResult, pErrorBound
	*			pProbCILeftBorder, pProbCIRightBorder.
	* pYesBitsetResult - the bitset containing states that satisfy the formula
	*
	* doSimHere - TRUE if this formula will be model checked using simulations
	* doSimBelow - TRUE if a subformula will be model checked using simulations
	* confidence - the confidence level for the given formula
	* pNoBitsetResult - the bitset containing states that so not satisfy the formula
	*			This one is needed for the case when simulations are used,
	*			because there we can have Yes/No/I do not know answers.
	* pProbCILeftBorder - an array of the left conf. int. borders
	* pProbCIRightBorder - an array of the right conf. int. borders
	*
	* error_bound - the single error bound for all pProbRewardResult values
	* pErrorBound - in case every result in pProbRewardResult has it's own error
	*		bound we should fill in the pErrorBound array with them
	* pProbRewardResult - the vector of doubles, the probability/rewards results
	*/
	typedef struct SFTypeRes{
		//The general formula-related fields
		int formula_type;
		char * pCharFormulaString;
		int prob_result_size;
		bitset * pYesBitsetResult;
		
		//The simulation related fiels
		int initial_state;
		BOOL isSimOneInitState;
		BOOL doSimHere;
		BOOL doSimBelow;
		double confidence;
		bitset * pNoBitsetResult;
		double * pProbCILeftBorder;
		double * pProbCIRightBorder;
		
		//The numerical-method related fields
		double error_bound;
		double * pErrorBound;
		double * pProbRewardResult;
	} TFTypeRes;
	typedef TFTypeRes* PTFTypeRes;
	
	//Any function that takes BOOL, PTFTypeRes type arguments and returns BOOL
	typedef BOOL ( * TPFunctOnPTFTypeRes ) (BOOL, PTFTypeRes );
	
	/**
	* This is the leaf state formula (ATOMIC_SF): tt, ff, AP (atomic proposition)
	* I.e it does not have a subformulas. Therefore it is treated as an atomic thing.
	* atomic_type - the formula type, can be one of:
	*	ATOMIC_SF_AP, ATOMIC_SF_TT, ATOMIC_SF_FF
	* pName - is the AP (label) name or NAME_FALSE, NAME_TRUE
	*/
	typedef struct SAtomicF{
		TFTypeRes type_res; //WARNIGN: MUST BE THE FIRST FIELD
		int atomic_type;
		char * pName;
	} TAtomicF;
	typedef TAtomicF* PTAtomicF;
	
	//Any function that takes BOOL, PTAtomicF type arguments and returns BOOL
	typedef BOOL ( * TPFunctOnPTAtomicF ) ( BOOL, PTAtomicF );
	
	/**
	* This is a unary operator for the state formulas (UNARY_OP_SF): (), !
	* type_res - contains the formula type and results.
	* unary_type - contains a type of the operator, that can be one of:
	*	UNARY_OP_SF_NEGATION, UNARY_OP_SF_BRACES
	* pSubForm - the pointer for the subformula here:
	*	
	*/
	typedef struct SUnaryOp{
		TFTypeRes type_res; //WARNIGN: MUST BE THE FIRST FIELD
		int unary_type;
		void * pSubForm;
	} TUnaryOp;
	typedef TUnaryOp* PTUnaryOp;
	
	//Any function that takes BOOL, PTUnaryOp type arguments and returns BOOL
	typedef BOOL ( * TPFunctOnPTUnaryOp ) ( BOOL, PTUnaryOp );
	
	/**
	* This is comparator formula (COMPARATOR_SF), a wrapper for the : X, S, L, P, E, C, Y operators
	* unary_op - the unary operator part of the simple formula, where the
	*		unary_type - contains a type of the operator, that can be one of:
	*			COMPARATOR_SF_LESS, COMPARATOR_SF_LESS_OR_EQUAL,
	*			COMPARATOR_SF_GREATER, COMPARATOR_SF_GREATER_OR_EQUAL,
	*			COMPARATOR_SF_REWARD_INTERVAL.
	* val_bound_left - the left prob./reward constraint
	* val_bound_right - the right reward constraint
	*NOTE: For all cases except COMPARATOR_SF_REWARD_INTERVAL we have val_bound_left
	* containing the probability bound
	*/
	typedef struct SCompStateF{
		TUnaryOp unary_op; //WARNIGN: MUST BE THE FIRST FIELD
		double val_bound_left;
		double val_bound_right;
	} TCompStateF;
	typedef TCompStateF* PTCompStateF;
	
	//Any function that takes BOOL, PTCompStateF type arguments and returns BOOL
	typedef BOOL ( * TPFunctOnPTCompStateF ) ( BOOL, PTCompStateF );
	
	/**
	* This is a steady-state/long-run formula (LONG_STEADY_F): L, S
	* Without the probability constraint.
	* unary_op.type_res - contains the formula type and results.
	* unary_op.unary_type - contains a type of the operator, that can be one of:
	*	LONG_STEADY_F_LONG_RUN, LONG_STEADY_F_STEADY_STATE
	* unary_op.pSubForm - the pointer for the subformula here:
	* pCompStateF - contains the pointer to the outer comparator operator
	*			It is needed for the simulation engine 
	*/
	typedef struct SLongSteadyF{
		TUnaryOp unary_op; //WARNIGN: MUST BE THE FIRST FIELD
		PTCompStateF pCompStateF;
	} TLongSteadyF;
	typedef TLongSteadyF* PTLongSteadyF;
	
	//Any function that takes BOOL, PTLongSteadyF type arguments and returns BOOL
	typedef BOOL ( * TPFunctOnPTLongSteadyF ) ( BOOL, PTLongSteadyF );
	
	/**
	* This is a binary operator for the state formulas (BINARY_OP_SF): &&, ||, =>
	* type_res - contains the formula type and results.
	* binary_type - contains a type of the operator, that can be one of:
	*	BINARY_OP_SF_IMPLIES, BINARY_OP_SF_AND, BINARY_OP_SF_OR
	* pSubFormL, pSubFormR - the pointers for the left and right subformulas here:
	*	
	*/
	typedef struct SBinaryOp{
		TFTypeRes type_res; //WARNIGN: MUST BE THE FIRST FIELD
		int binary_type;
		void * pSubFormL;
		void * pSubFormR;
	} TBinaryOp;
	typedef TBinaryOp* PTBinaryOp;
	
	//Any function that takes BOOL, BOOL, PTBinaryOp type arguments and returns BOOL
	typedef BOOL ( * TPFunctOnPTBinaryOp ) ( BOOL, BOOL, PTBinaryOp );
	
	/**
	* This is a pure reward-oriented state formula (PURE_REWARD_SF): C, E, Y
	* without the reward bounds that have to be defined in the form of TCompStateF.
	* unary_op - the unary operator part of the simple formula, where the
	*		unary_type - contains a type of the operator, that can be one of:
	*			PURE_REWARD_SF_EXPECTED_RR, PURE_REWARD_SF_INSTANT_R,
	*			PURE_REWARD_SF_EXPECTED_AR
	* time - the time instance
	*/
	typedef struct SPureRewardF{
		TUnaryOp unary_op; //WARNIGN: MUST BE THE FIRST FIELD
		double time;
	} TPureRewardF;
	typedef TPureRewardF* PTPureRewardF;
	
	//Any function that takes BOOL, PTPureRewardF type arguments and returns BOOL
	typedef BOOL ( * TPFunctOnPTPureRewardF ) ( BOOL, PTPureRewardF );
	
	/**
	* This is the next formula (NEXT_PF): X
	* unary_op - the unary operator part of the formula, where the
	*		unary_type - contains a type of the operator, that can be one of:
	*		NEXT_PF_UNB, NEXT_PF_TIME, NEXT_PF_TIME_REWARD
	* left_time_bound - the lower time bound
	* right_time_bound - the upper time bound
	* left_reward_bound - the lower reward bound
	* right_reward_bound - the upper reward bound
	* pProbRewardResult - the resulting array of probabilities/rewards
	* prob_result_size - the size of the pProbRewardResult
	*/
	typedef struct SNextF{
		TUnaryOp unary_op; //WARNIGN: MUST BE THE FIRST FIELD
		double left_time_bound;
		double right_time_bound;
		double left_reward_bound;
		double right_reward_bound;
	} TNextF;
	typedef TNextF* PTNextF;
	
	//Any function that takes BOOL, PTNextF type arguments and returns BOOL
	typedef BOOL ( * TPFunctOnPTNextF ) ( BOOL, PTNextF );
	
	/**
	* This is the until formula (UNTIL_PF): U
	* binary_op - the binary operator part of the formula, where the
	*		binary_type - contains a type of the operator, that can be one of:
	*			UNTIL_PF_UNB, UNTIL_PF_TIME, UNTIL_PF_TIME_REWARD
	* pCompStateF - contains the pointer to the outer comparator operator
	*			It is needed for the simulation engine 
	* left_time_bound - the lower time bound
	* right_time_bound - the upper time bound
	* left_reward_bound - the lower reward bound
	* right_reward_bound - the upper reward bound
	* pProbRewardResult - the resulting array of probabilities/rewards
	* prob_result_size - the size of the pProbRewardResult
	*/
	typedef struct SUntilF{
		TBinaryOp binary_op; //WARNIGN: MUST BE THE FIRST FIELD
		PTCompStateF pCompStateF;
		double left_time_bound;
		double right_time_bound;
		double left_reward_bound;
		double right_reward_bound;
	} TUntilF;
	typedef TUntilF* PTUntilF;
	
	//Any function that takes BOOL, BOOL, PTUntilF type arguments and returns BOOL
	typedef BOOL ( * TPFunctOnPTUntilF ) ( BOOL, BOOL, PTUntilF );
	
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
				TPFunctOnPTUntilF pFPTUntilF);
	
	/**
	* Creates an atomic formula structure (ATOMIC_SF) such as an atomic
	* proposition (AP), TRUE (TT) or FALSE (FF)
	* @param label contain one of: AP, NAME_FALSE, NAME_TRUE
	* @param atomic_type one of: ATOMIC_SF_AP, ATOMIC_SF_TT, ATOMIC_SF_FF
	* @return returns the pointer to the newly created TAtomicF node casted to (void *)
	*/
	extern void * allocateAtomicFormula(char * label, const int atomic_type);
	
	/**
	* Creates a "unary operation on a subformula" structure (UNARY_OP_SF)
	* such as braces (pSubForm) or negation !pSubForm
	* @param pSubForm the subformula for the unary operator
	* @param unary_type one of: UNARY_OP_SF_NEGATION, UNARY_OP_SF_BRACES
	* @return returns the pointer to the newly created TUnaryOp node casted to (void *)
	*/
	extern void * allocateUnaryOperatorFormula(void * pSubForm, const int unary_type);
	
	/**
	* Creates a "binary operation on a subformula" structure (BINARY_OP_SF)
	* such as and pSubFormL&&pSubFormR, or pSubFormL||pSubFormR, implication pSubFormL=>pSubFormR
	* @param pSubFormL the left subformula for the binary operator
	* @param pSubFormR the right subformula for the binary operator
	* @param unary_type one of: BINARY_OP_SF_IMPLIES, BINARY_OP_SF_AND, BINARY_OP_SF_OR
	* @return returns the pointer to the newly created TUnaryOp node casted to (void *)
	*/
	extern void * allocateBinaryOperatorFormula(void * pSubFormL, void * pSubFormR, const int binary_type);
	
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
	extern void * allocateComparatorFormula(double val_bound_left, double val_bound_right, void * pSubForm,
						const int unary_type);
	
	/**
	* Creates a steady-state/long-run formula node (LONG_STEADY_F): L, S
	* Without the probability constraint.
	* @param pSubForm the pointer for the subformula
	* @param unary_type one of: LONG_STEADY_F_LONG_RUN, LONG_STEADY_F_STEADY_STATE
	* @return returns the pointer to the newly created TLongSteadyF node casted to (void *)
	*/
	extern void * allocateLongSteadyFormula(void * pSubForm, const int unary_type);
	
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
	extern void * allocatePureRewardFormula(double time, void * pSubForm, const int unary_type);
	
	/**
	* Creates a next-formula structure (NEXT_PF) such as X operator of PCTL, CSL
	* @param left_time_bound the lower time bound
	* @param right_time_bound the upper time bound
	* @param left_reward_bound the left reward constraint
	* @param right_reward_bound the right reward constraint
	* @param pSubForm the subformula for the until formula
	* @param unary_type one of: NEXT_PF_UNB, NEXT_PF_TIME, NEXT_PF_TIME_REWARD
	* @return returns the pointer to the newly created PTNextF node casted to (void *)
	*/
	extern void * allocateNextPathFormula(double left_time_bound, double right_time_bound, double left_reward_bound,
						double right_reward_bound, void * pSubForm, const int unary_type);
	
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
	extern void * allocateUntilPathFormula(double left_time_bound, double right_time_bound,
						double left_reward_bound, double right_reward_bound,
						void * pSubFormL, void * pSubFormR, const int binary_type);
	
	/**
	* This method deallocates memory allocated for the tree which has the given root node.
	* @param pFormulaTreeNode the root tree node scheduled for deallocation
	*/
	extern void freeFormulaTree(void * pFormulaTreeNode);
	
	/**
	* This is a dedicated function for printing the node results and main parameters
	* @param pFTypeRes the node to print
	*/
	void printFormulaBitsetResultParams(PTFTypeRes pFTypeRes);
	
	/**
	* This method prints for the tree which has the given root node.
	* @param pFormulaTreeNode the root tree node scheduled for printing
	*/
	extern void dumpFormulaTree(void * pFormulaTreeNode);
	
#endif
