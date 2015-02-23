%{
/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: mrmc_grammar.y,v $
*	$Revision: 1.38 $
*	$Date: 2007/10/14 15:11:58 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2007.
*	Authors: Maneesh Khattri, Ivan Zapreev
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
*	Source description: This is a parser for the logic CSL/PCTL/PRCTL/CSRL in MCC.
*/

#include "macro.h"
#include "runtime.h"
#include "parser_to_core.h"
#include "parser_to_tree.h"

#define YYERROR_VERBOSE 1

extern int yylex(void);
%}
%union
{
	BOOL bval;
	int ival;
	char *sval;
	float fval;
	double dval;
	int nval;
	void *formula_tree_node;
};

%nonassoc <sval> ATOMIC_PROPOSITION
%nonassoc <dval> DOUBLE_VALUE

%nonassoc LEFT_PARENTHESIS RIGHT_PARENTHESIS LEFT_CURLY_BRACKET RIGHT_CURLY_BRACKET LEFT_SQUARE_BRACKET RIGHT_SQUARE_BRACKET GREATER GREATER_OR_EQUAL LESS LESS_OR_EQUAL PRINT TREE ERROR_BOUND OVERFLOW_VAL UNDERFLOW_VAL METHOD_PATH METHOD_STEADY METHOD_BSCC COMMA COMPLEMENT QUIT SET STEADY_STATE_F GAUSS_JACOBI_M GAUSS_SEIDEL_M RECURSIVE_M NON_RECURSIVE_M MAX_ITERATIONS METHOD_UNTIL_REWARDS UNIFORMIZATION_SERICOLA UNIFORMIZATION_QURESHI_SANDERS DISCRETIZATION_TIJMS_VELDMAN SSD ON OFF RESULT STATE SIMULATION SIM_METHOD_STEADY SIM_PURE_MODE SIM_HYBRID_MODE SIM_TYPE SIM_TYPE_ONE SIM_TYPE_ALL SIM_GEN_CONF SIM_CONV_CONF LOGIC REWARDS COMMON MAX_SAMPLE_SIZE MIN_SAMPLE_SIZE SAMPLE_SIZE_DELTA MAX_SIM_DEPTH MIN_SIM_DEPTH SIM_DEPTH_DELTA SIM_METHOD_DISC SIM_METHOD_EXP RNG_APP_CRYPT RNG_PRISM RNG_CIARDO RNG_YMER RNG_GSL_RANLUX RNG_GSL_LFG RNG_GSL_TAUS INITIAL_STATE

%nonassoc PROBABILITY_F NEXT_F UNTIL_F SPC NEWLINE TTRUE FFALSE IMPLIES EXPECTED_REWARD_RATE_F INSTANTANEOUS_REWARD_F EXPECTED_ACCUMULATED_REWARD_F LONG_RUN_F HELP PROB_THRESHOLD_QURESHI_SANDERS DISCRETIZATION_FACTOR

%left AND OR
%right NOT

%type <bval> on_off
%type <ival> mformula comparator rng_method
%type <formula_tree_node> stateformula termformula factorformula steadyformula longrunformula pathformula eformula cformula yformula untilformula nextformula
/*The comparator is for {||, &&, >, <, =>, <=}*/
	
%destructor { freeFormulaTree($$); } stateformula termformula factorformula steadyformula longrunformula pathformula eformula cformula yformula untilformula nextformula
			
/**********************THE BEGINNING OF MAIN INTERFACE COMMANDS*********************/
			
%%
mformula		: NEWLINE
			{
				return 1;
			}
			//WARNING: This is the top most for the FORMULA TREE!
			| stateformula NEWLINE
			{
				modelCheckFormulaTree( $1 );
				
				return 1;
			}
			| QUIT NEWLINE
			{
				clearOldModelCheckingResults();
				return 0;
			}
/********************************************************************************/
/*******************PRINTING HELP MESSAGES FOR DIFFERENT SUB-PARTS***************/
/********************************************************************************/
			| HELP NEWLINE
			{
				printHelpMessage(HELP_GENERAL_MSG_TYPE);
				return 1;
			}
			| HELP COMMON NEWLINE
			{
				printHelpMessage(HELP_COMMON_MSG_TYPE);
				return 1;
			}
			| HELP LOGIC NEWLINE
			{
				printHelpMessage(HELP_LOGIC_MSG_TYPE);
				return 1;
			}
			| HELP REWARDS NEWLINE
			{
				printHelpMessage(HELP_REWARDS_MSG_TYPE);
				return 1;
			}
			| HELP SIMULATION NEWLINE
			{
				printHelpMessage(HELP_SIMULATION_MSG_TYPE);
				return 1;
			}
/********************************************************************************/
/*******************SET THE GENERAL-COMMON PARAMETERS****************************/
/********************************************************************************/
			| SET SSD on_off NEWLINE
			{
				set_ssd($3);
				return 1;
			}
			| SET METHOD_PATH GAUSS_JACOBI_M NEWLINE
			{
				set_method_path(GJ);
				return 1;
			}
			| SET METHOD_PATH GAUSS_SEIDEL_M NEWLINE
			{
				set_method_path(GS);
				return 1;
			}
			| SET METHOD_STEADY GAUSS_JACOBI_M NEWLINE
			{
				set_method_steady(GJ);
				return 1;
			}
			| SET METHOD_STEADY GAUSS_SEIDEL_M NEWLINE
			{
				set_method_steady(GS);
				return 1;
			}
			| SET METHOD_BSCC RECURSIVE_M NEWLINE
			{
				set_method_bscc(REC);
				return 1;
			}
			| SET METHOD_BSCC NON_RECURSIVE_M NEWLINE
			{
				set_method_bscc(NON_REC);
				return 1;
			}		
/********************************************************************************/
/******************SET THE PRINTING RELATED PARAMETERS***************************/
/********************************************************************************/
			| SET PRINT on_off NEWLINE
			{
				setPrintingStatus($3);
				return 1;
			}
			| PRINT NEWLINE
			{
				print_runtime_info();
				return 1;
			}
			| PRINT TREE NEWLINE
			{
				printResultFormulaTree();
				return 1;
			}
			| RESULT LEFT_SQUARE_BRACKET DOUBLE_VALUE RIGHT_SQUARE_BRACKET NEWLINE
			{
				printResultingStateProbability( (int) $3);
				return 1;
			}
			| STATE LEFT_SQUARE_BRACKET DOUBLE_VALUE RIGHT_SQUARE_BRACKET NEWLINE
			{
				printResultingStateSatisfyability( (int) $3 );
				return 1;
			}
/********************************************************************************/
/*****************SET THE SIMULATION RELATED PARAMETERS**************************/
/********************************************************************************/
			| SET SIM_METHOD_STEADY SIM_PURE_MODE NEWLINE
			{
				setSimSteadyStateModePure( TRUE );
				return 1;
			}
			| SET SIM_METHOD_STEADY SIM_HYBRID_MODE NEWLINE
			{
				setSimSteadyStateModePure( FALSE );
				return 1;
			}
			| SET SIMULATION on_off NEWLINE
			{
				setSimulationStatus($3);
				return 1;
			}
			| SET SIM_TYPE SIM_TYPE_ONE NEWLINE
			{
				setSimOneInitState( TRUE );
				return 1;
			}
			| SET SIM_TYPE SIM_TYPE_ALL NEWLINE
			{
				setSimOneInitState( FALSE );
				return 1;
			}
			| SET SIM_GEN_CONF DOUBLE_VALUE NEWLINE
			{
				setSimGeneralConfidence($3);
				return 1;
			}
			| SET SIM_CONV_CONF DOUBLE_VALUE NEWLINE
			{
				setSimConvConfidence($3);
				return 1;
			}
			| SET SIM_METHOD_EXP rng_method NEWLINE
			{
				setRNGMethodExp($3);
				return 1;
			}
			| SET SIM_METHOD_DISC rng_method NEWLINE
			{
				setRNGMethodDiscrete($3);
				return 1;
			}
			| SET MAX_SAMPLE_SIZE DOUBLE_VALUE NEWLINE
			{
				setSimMaxSampleSize( (int) $3);
				return 1;
			}
			| SET MIN_SAMPLE_SIZE DOUBLE_VALUE NEWLINE
			{
				setSimMinSampleSize( (int) $3 );
				return 1;
			}
			| SET SAMPLE_SIZE_DELTA DOUBLE_VALUE NEWLINE
			{
				setSimSampleSizeStep( (int) $3 );
				return 1;
			}
			| SET MAX_SIM_DEPTH DOUBLE_VALUE NEWLINE
			{
				setSimMaxSimulationDepth( (int) $3 );
				return 1;
			}
			| SET MIN_SIM_DEPTH DOUBLE_VALUE NEWLINE
			{
				setSimMinSimulationDepth( (int) $3 );
				return 1;
			}
			| SET SIM_DEPTH_DELTA DOUBLE_VALUE NEWLINE
			{
				setSimSimulationDepthStep( (int) $3 );
				return 1;
			}
			| SET INITIAL_STATE DOUBLE_VALUE NEWLINE
			{
				setSimInitialStateRuntime( (int) $3 );
				return 1;
			}
/********************************************************************************/
/*****************SET THE ITERATION METHOD RELATED PARAMETERS********************/
/********************************************************************************/
			| SET ERROR_BOUND DOUBLE_VALUE NEWLINE
			{
				set_error_bound($3);
				return 1;
			}
			| SET MAX_ITERATIONS DOUBLE_VALUE NEWLINE
			{
				set_max_iterations( (int) $3);
				return 1;
			}
			| SET OVERFLOW_VAL DOUBLE_VALUE NEWLINE
			{
				set_overflow($3);
				return 1;
			}
			| SET UNDERFLOW_VAL DOUBLE_VALUE NEWLINE
			{
				set_underflow($3);
				return 1;
			}
/********************************************************************************/
/*****************SET THE REWARD EXTENTION RELATED PARAMETERS********************/
/********************************************************************************/
			| SET PROB_THRESHOLD_QURESHI_SANDERS DOUBLE_VALUE NEWLINE
			{
				set_w($3);
				return 1;
			}
			| SET DISCRETIZATION_FACTOR DOUBLE_VALUE NEWLINE
			{
				set_d_factor($3);
				return 1;
			}
			| SET METHOD_UNTIL_REWARDS UNIFORMIZATION_SERICOLA NEWLINE
			{
				set_method_until_rewards(US);
				return 1;
			}
			| SET METHOD_UNTIL_REWARDS UNIFORMIZATION_QURESHI_SANDERS NEWLINE
			{
				set_method_until_rewards(UQS);
				return 1;
			}
			| SET METHOD_UNTIL_REWARDS DISCRETIZATION_TIJMS_VELDMAN NEWLINE
			{
				set_method_until_rewards(DTV);
				return 1;
			}
			;
/**********************THE END OF MAIN INTERFACE COMMANDS*********************/
			
/******************THE BEGINNING OF GRAMMAR FOR FORMULAS******************/
			/*RESULT IN A BITSET OF SATISFYING STATES*/
			
stateformula		: termformula
			{
				$$ = $1;
			}
			| stateformula IMPLIES termformula
			{
				$$ = allocateBinaryOperatorFormula( $1, $3, BINARY_OP_SF_IMPLIES );
			}
			| stateformula OR termformula
			{
				$$ = allocateBinaryOperatorFormula( $1, $3, BINARY_OP_SF_OR );
			}
termformula		: factorformula
			{
				$$ = $1;
			}
			| termformula AND factorformula
			{
				$$ = allocateBinaryOperatorFormula( $1, $3, BINARY_OP_SF_AND );
			}
factorformula		: LEFT_PARENTHESIS stateformula RIGHT_PARENTHESIS
			{
				$$= allocateUnaryOperatorFormula( $2, UNARY_OP_SF_BRACES );
			}
			| COMPLEMENT factorformula
			{
				$$ = allocateUnaryOperatorFormula($2, UNARY_OP_SF_NEGATION );
			}
			| TTRUE
			{
				$$ = allocateAtomicFormula( NAME_TRUE, ATOMIC_SF_TT );
			}
			| FFALSE
			{
				$$ = allocateAtomicFormula( NAME_FALSE, ATOMIC_SF_FF );
			}
			| ATOMIC_PROPOSITION
			{
				$$ = allocateAtomicFormula( $1, ATOMIC_SF_AP );
			}
			| steadyformula
			{
				$$ = $1;
			}
			| longrunformula
			{
				$$ = $1;
			}
			| pathformula
			{
				$$ = $1;
			}
			| eformula
			{
				$$ = $1;
			}
			| cformula
			{
				$$ = $1;
			}
			| yformula
			{
				$$ = $1;
			}
			
/***************The complex formulas, treated as "atomic", are listed below***************/
			/*RESULT IN A BITSET OF SATISFYING STATES
			* Parameters:
			*	comparator - The comparator, <,>,<=,>=
			*	DOUBLE_VALUE - the probability bound
			*	stateformula - the subformula
			*/
			
steadyformula		: STEADY_STATE_F LEFT_CURLY_BRACKET comparator DOUBLE_VALUE RIGHT_CURLY_BRACKET
			  LEFT_SQUARE_BRACKET stateformula RIGHT_SQUARE_BRACKET 
			{
				void * pLongSteadyF = allocateLongSteadyFormula($7, LONG_STEADY_F_STEADY_STATE);
				$$ = allocateComparatorFormula( $4, 0, pLongSteadyF, $3 );
			}
			;
longrunformula		: LONG_RUN_F LEFT_CURLY_BRACKET comparator DOUBLE_VALUE RIGHT_CURLY_BRACKET
			  LEFT_SQUARE_BRACKET stateformula RIGHT_SQUARE_BRACKET 
			{
				void * pLongSteadyF = allocateLongSteadyFormula($7, LONG_STEADY_F_LONG_RUN);
				$$ = allocateComparatorFormula( $4, 0, pLongSteadyF, $3 );
			}
			;
pathformula		: PROBABILITY_F LEFT_CURLY_BRACKET comparator DOUBLE_VALUE RIGHT_CURLY_BRACKET
			  LEFT_SQUARE_BRACKET untilformula RIGHT_SQUARE_BRACKET
			{
				$$ = allocateComparatorFormula( $4, 0, $7, $3 );
			}
			| PROBABILITY_F LEFT_CURLY_BRACKET comparator DOUBLE_VALUE RIGHT_CURLY_BRACKET
			  LEFT_SQUARE_BRACKET nextformula RIGHT_SQUARE_BRACKET
			{
				$$ = allocateComparatorFormula( $4, 0, $7, $3 );
			}
			;
			
/**************************The pure REWARD-related operators**************************/
			/*RESULT IN A BITSET OF SATISFYING STATES
			* Parameters:
			*	DOUBLE_VALUE - the numerical bound
			*	DOUBLE_VALUE - the numerical bound
			*	DOUBLE_VALUE - the numerical bound
			*	stateformula - the subformula
			*/
			
eformula		: EXPECTED_REWARD_RATE_F
			  LEFT_SQUARE_BRACKET DOUBLE_VALUE COMMA DOUBLE_VALUE RIGHT_SQUARE_BRACKET
			  LEFT_SQUARE_BRACKET stateformula RIGHT_SQUARE_BRACKET
			{
				void * pPureRewardF = allocatePureRewardFormula( 0 , $8, PURE_REWARD_SF_EXPECTED_RR );
				$$ = allocateComparatorFormula( $3, $5, pPureRewardF, COMPARATOR_SF_REWARD_INTERVAL );
			}
			| EXPECTED_REWARD_RATE_F
			  LEFT_SQUARE_BRACKET DOUBLE_VALUE RIGHT_SQUARE_BRACKET
			  LEFT_SQUARE_BRACKET DOUBLE_VALUE COMMA DOUBLE_VALUE RIGHT_SQUARE_BRACKET
			  LEFT_SQUARE_BRACKET stateformula RIGHT_SQUARE_BRACKET
			{
				void * pPureRewardF = allocatePureRewardFormula( $3 , $11, PURE_REWARD_SF_EXPECTED_RR );
				$$ = allocateComparatorFormula( $6, $8, pPureRewardF, COMPARATOR_SF_REWARD_INTERVAL );
			}
			;
cformula		: INSTANTANEOUS_REWARD_F
			  LEFT_SQUARE_BRACKET DOUBLE_VALUE RIGHT_SQUARE_BRACKET
			  LEFT_SQUARE_BRACKET DOUBLE_VALUE COMMA DOUBLE_VALUE RIGHT_SQUARE_BRACKET
			  LEFT_SQUARE_BRACKET stateformula RIGHT_SQUARE_BRACKET
			{
				void * pPureRewardF = allocatePureRewardFormula( $3 , $11, PURE_REWARD_SF_INSTANT_R );
				$$ = allocateComparatorFormula( $6, $8, pPureRewardF, COMPARATOR_SF_REWARD_INTERVAL );
			}
			;
yformula		: EXPECTED_ACCUMULATED_REWARD_F
			  LEFT_SQUARE_BRACKET DOUBLE_VALUE RIGHT_SQUARE_BRACKET
			  LEFT_SQUARE_BRACKET DOUBLE_VALUE COMMA DOUBLE_VALUE RIGHT_SQUARE_BRACKET
			  LEFT_SQUARE_BRACKET stateformula RIGHT_SQUARE_BRACKET 
			{
				void * pPureRewardF = allocatePureRewardFormula( $3 , $11, PURE_REWARD_SF_EXPECTED_AR );
				$$ = allocateComparatorFormula( $6, $8, pPureRewardF, COMPARATOR_SF_REWARD_INTERVAL );
			}
			;
			
/*****************The Path formulas, such as Until and Next*****************/
			
			/*RESULT IN AN ARRAY OF PROBABILITY VALUES
			* Parameters:
			*	stateformula - the subformula
			*/
nextformula		: NEXT_F stateformula 
			{
				$$ = allocateNextPathFormula( 0, 0, 0, 0, $2, NEXT_PF_UNB);
			}
			/* Parameters:
			*	DOUBLE_VALUE - the lower time bound
			*	DOUBLE_VALUE - the upper time bound
			*	stateformula - the subformula
			*/
			| NEXT_F LEFT_SQUARE_BRACKET DOUBLE_VALUE COMMA DOUBLE_VALUE RIGHT_SQUARE_BRACKET stateformula 
			{
				$$ = allocateNextPathFormula( $3, $5, 0, 0, $7, NEXT_PF_TIME);
			}
			/* Parameters:
			*	DOUBLE_VALUE - the lower time bound
			*	DOUBLE_VALUE - the upper time bound
			*	DOUBLE_VALUE - the lower reward bound
			*	DOUBLE_VALUE - the upper reward bound
			*	stateformula - the subformula
			*/
			| NEXT_F LEFT_SQUARE_BRACKET DOUBLE_VALUE COMMA DOUBLE_VALUE RIGHT_SQUARE_BRACKET
			  LEFT_SQUARE_BRACKET DOUBLE_VALUE COMMA DOUBLE_VALUE RIGHT_SQUARE_BRACKET stateformula
			{
				$$ = allocateNextPathFormula( $3, $5, $8, $10, $12, NEXT_PF_TIME_REWARD);
			}
			;
			
			/*IN CASE OF SIMULATIONS RESULT IN AN ARRAY OF CONF. INT.
			* Parameters:
			*	stateformula - the left subformula
			*	stateformula - the right subformula
			*/
untilformula		: stateformula UNTIL_F stateformula
			{
				$$ = allocateUntilPathFormula( 0, 0, 0, 0, $1, $3, UNTIL_PF_UNB);
			}
			/* Parameters:
			*	stateformula - the left subformula
			*	DOUBLE_VALUE - the lower time bound
			*	DOUBLE_VALUE - the upper time bound
			*	stateformula - the right subformula
			*/
			| stateformula UNTIL_F LEFT_SQUARE_BRACKET DOUBLE_VALUE COMMA DOUBLE_VALUE RIGHT_SQUARE_BRACKET stateformula
			{
				$$ = allocateUntilPathFormula( $4, $6, 0, 0, $1, $8, UNTIL_PF_TIME);
			}
			/* Parameters:
			*	stateformula - the left subformula
			*	DOUBLE_VALUE - the lower time bound
			*	DOUBLE_VALUE - the upper time bound
			*	DOUBLE_VALUE - the lower reward bound
			*	DOUBLE_VALUE - the upper reward bound
			*	stateformula - the right subformula
			*/
			| stateformula UNTIL_F LEFT_SQUARE_BRACKET DOUBLE_VALUE COMMA DOUBLE_VALUE RIGHT_SQUARE_BRACKET
			  LEFT_SQUARE_BRACKET DOUBLE_VALUE COMMA DOUBLE_VALUE RIGHT_SQUARE_BRACKET stateformula
			{
				$$ = allocateUntilPathFormula( $4, $6, $9, $11, $1, $13, UNTIL_PF_TIME_REWARD);
			}
			;
			
/*****************The comparators*****************/
			
comparator		: GREATER
			{
				$$ = COMPARATOR_SF_GREATER;
			}
			| GREATER_OR_EQUAL
			{
				$$ = COMPARATOR_SF_GREATER_OR_EQUAL;
			}
			| LESS
			{
				$$ = COMPARATOR_SF_LESS;
			}
			| LESS_OR_EQUAL
			{
				$$ = COMPARATOR_SF_LESS_OR_EQUAL;
			}
			;
			
/*****************The on/off boolean result*****************/
			
on_off		: ON
		{
			$$ = TRUE;
		}
		| OFF
		{
			$$  = FALSE;
		}
		;
		
/*****************The Random number generator method*****************/
		
rng_method	: RNG_APP_CRYPT
		{
			$$ = RNG_APP_CRYPT_METHOD;
		}
		| RNG_PRISM
		{
			$$ = RNG_PRISM_METHOD;
		}
		| RNG_CIARDO
		{
			$$ = RNG_CIARDO_METHOD;
		}
		| RNG_YMER
		{
			$$ = RNG_YMER_METHOD;
		}
		| RNG_GSL_RANLUX
		{
			$$ = RNG_GSL_RANLUX_METHOD;
		}
		| RNG_GSL_LFG
		{
			$$ = RNG_GSL_LFG_METHOD;
		}
		| RNG_GSL_TAUS
		{
			$$ = RNG_GSL_TAUS_METHOD;
		}
		;
/******************THE END OF GRAMMAR FOR FORMULAS******************/
			
%%
yyerror(char *s)
{
	printf("%s\n",s);
	//NOTE: I am not quite sure we have to discard the
	//old results in case of the syntax error in the
	//input command or formula
	//clearOldModelCheckingResults();
}

yywrap()
{
}
