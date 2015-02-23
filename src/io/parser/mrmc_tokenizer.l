%{
/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: mrmc_tokenizer.l,v $
*	$Revision: 1.15 $
*	$Date: 2007/10/24 13:39:49 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
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
*	Source description: This is a lexical analyzer for the logic PCTL/CSL/PRCTL/CSRL in MRMC.
*/

#include<stdio.h>
#include<stdlib.h>
#include "foxglynn.h"
#include "bitset.h"
#include "sparse.h"
#include "label.h"
#include "runtime.h"
#include "mrmc_grammar.tab.h"
#define prc(name) name=='y'
char pr='n';
%}

let      [_a-zA-Z]
num      [0-9]
alnum    [_a-zA-Z0-9<>_^*+-=]
white    [ \t\n\r\b]
%%

({num}+|{num}+\.{num}*|{num}*\.{num}+)([eE][+-]?{num}+)?[dD]? {if(prc(pr)) printf("DOUBLE_VALUE  : %s \n" ,yytext); yylval.dval=atof(yytext); return DOUBLE_VALUE;}

"("		{ if(prc(pr)) printf("LEFT_PARENTHESIS : %s\n", yytext); return LEFT_PARENTHESIS;}
")"		{ if(prc(pr)) printf("RIGHT_PARENTHESIS : %s\n",yytext); return RIGHT_PARENTHESIS;}
"{"		{ if(prc(pr)) printf("LEFT_CURLY_BRACKET : %s\n",yytext); return LEFT_CURLY_BRACKET;}
"}"		{ if(prc(pr)) printf("RIGHT_CURLY_BRACKET : %s\n",yytext); return RIGHT_CURLY_BRACKET;}
"["		{ if(prc(pr)) printf("LEFT_SQUARE_BRACKET : %s\n",yytext); return LEFT_SQUARE_BRACKET;}
"]"		{ if(prc(pr)) printf("RIGHT_SQUARE_BRACKET : %s\n",yytext); return RIGHT_SQUARE_BRACKET;}
">"		{ if(prc(pr)) printf("GREATER : %s\n",yytext); return GREATER;}
">="		{ if(prc(pr)) printf("GREATER_OR_EQUAL : %s\n",yytext); return GREATER_OR_EQUAL;}
"<"		{ if(prc(pr)) printf("LESS : %s\n",yytext); return LESS;}
"<="		{ if(prc(pr)) printf("LESS_OR_EQUAL : %s\n",yytext); return LESS_OR_EQUAL;}
","		{ if(prc(pr)) printf("COMMA : %s\n",yytext); return COMMA;}
"&&"		{ if(prc(pr)) printf("AND	   : %s\n",yytext); return AND;}
"||"		{ if(prc(pr)) printf("OR      : %s\n",yytext); return OR;}
"=>"		{ if(prc(pr)) printf("IMPLIES   : %s\n",yytext); return IMPLIES;}
"!"		{ if(prc(pr)) printf("COMPLEMENT   : %s\n",yytext); return COMPLEMENT;}

"print"		{ if(prc(pr)) printf("PRINT   : %s\n",yytext); return PRINT;}
"tree"		{ if(prc(pr)) printf("TREE   : %s\n",yytext); return TREE;}

"set"		{ if(prc(pr)) printf("SET   : %s\n",yytext); return SET;}
"w"		{ if(prc(pr)) printf("PROB_THRESHOLD_QURESHI_SANDERS   : %s\n",yytext); return PROB_THRESHOLD_QURESHI_SANDERS;}
"d"		{ if(prc(pr)) printf("DISCRETIZATION_FACTOR   : %s\n",yytext); return DISCRETIZATION_FACTOR;}
"error_bound"	{ if(prc(pr)) printf("ERROR_BOUND   : %s\n",yytext); return ERROR_BOUND;}
"max_iter"	{ if(prc(pr)) printf("MAX_ITERATIONS   : %s\n",yytext); return MAX_ITERATIONS;}
"overflow"	{ if(prc(pr)) printf("OVERFLOW_VAL   : %s\n",yytext); return OVERFLOW_VAL;}
"underflow"	{ if(prc(pr)) printf("UNDERFLOW_VAL   : %s\n",yytext); return UNDERFLOW_VAL;}
"method_path"	{ if(prc(pr)) printf("METHOD_PATH   : %s\n",yytext); return METHOD_PATH;}
"method_steady"	{ if(prc(pr)) printf("METHOD_STEADY   : %s\n",yytext); return METHOD_STEADY;}
"method_bscc"	{ if(prc(pr)) printf("METHOD_BSCC   : %s\n",yytext); return METHOD_BSCC;}
"gauss_jacobi"	{ if(prc(pr)) printf("GAUSS_JACOBI_M   : %s\n",yytext); return GAUSS_JACOBI_M;}
"gauss_seidel"	{ if(prc(pr)) printf("GAUSS_SEIDEL_M   : %s\n",yytext); return GAUSS_SEIDEL_M;}
"recursive"	{ if(prc(pr)) printf("RECURSIVE_M    : %s\n",yytext); return RECURSIVE_M;}
"non_recursive"	{ if(prc(pr)) printf("NON_RECURSIVE_M    : %s\n",yytext); return NON_RECURSIVE_M;}
"method_until_rewards" { if(prc(pr)) printf("METHOD_UNTIL_REWARDS   : %s\n",yytext); return METHOD_UNTIL_REWARDS;}
"uniformization_sericola" { if(prc(pr)) printf("UNIFORMIZATION_SERICOLA   : %s\n",yytext); return UNIFORMIZATION_SERICOLA;}
"uniformization_qureshi_sanders" { if(prc(pr)) printf("UNIFORMIZATION_QURESHI_SANDERS   : %s\n",yytext); return UNIFORMIZATION_QURESHI_SANDERS;}
"discretization_tijms_veldman" { if(prc(pr)) printf("DISCRETIZATION_TIJMS_VELDMAN   : %s\n",yytext); return DISCRETIZATION_TIJMS_VELDMAN;}

"quit"		{ if(prc(pr)) printf("QUIT   : %s\n",yytext); return QUIT;}
"help"		{ if(prc(pr)) printf("HELP   : %s\n",yytext); return HELP;}
"ssd"		{ if(prc(pr)) printf("SSD   : %s\n",yytext); return SSD;}

"logic"		{ if(prc(pr)) printf("LOGIC   : %s\n",yytext); return LOGIC;}
"rewards"		{ if(prc(pr)) printf("REWARDS   : %s\n",yytext); return REWARDS;}
"common"		{ if(prc(pr)) printf("COMMON   : %s\n",yytext); return COMMON;}

"simulation"	{ if(prc(pr)) printf("SIMULATION   : %s\n",yytext); return SIMULATION;}
"sim_method_steady"	{ if(prc(pr)) printf("SIM_METHOD_STEADY   : %s\n",yytext); return SIM_METHOD_STEADY;}
"sim_type"	{ if(prc(pr)) printf("SIM_TYPE   : %s\n",yytext); return SIM_TYPE;}
"one"		{ if(prc(pr)) printf("SIM_TYPE_ONE   : %s\n",yytext); return SIM_TYPE_ONE;}
"all"		{ if(prc(pr)) printf("SIM_TYPE_ALL   : %s\n",yytext); return SIM_TYPE_ALL;}
"pure"		{ if(prc(pr)) printf("SIM_PURE_MODE   : %s\n",yytext); return SIM_PURE_MODE;}
"hybrid"	{ if(prc(pr)) printf("SIM_HYBRID_MODE   : %s\n",yytext); return SIM_HYBRID_MODE;}
"gen_conf"	{ if(prc(pr)) printf("SIM_GEN_CONF   : %s\n",yytext); return SIM_GEN_CONF;}
"conv_conf"	{ if(prc(pr)) printf("SIM_CONV_CONF   : %s\n",yytext); return SIM_CONV_CONF;}
"initial_state"	{ if(prc(pr)) printf("INITIAL_STATE  : %s\n",yytext); return INITIAL_STATE;}
"max_sample_size"	{ if(prc(pr)) printf("MAX_SAMPLE_SIZE  : %s\n",yytext); return MAX_SAMPLE_SIZE;}
"min_sample_size"	{ if(prc(pr)) printf("MIN_SAMPLE_SIZE   : %s\n",yytext); return MIN_SAMPLE_SIZE;}
"sample_size_step"	{ if(prc(pr)) printf("SAMPLE_SIZE_DELTA   : %s\n",yytext); return SAMPLE_SIZE_DELTA;}
"max_sim_depth"		{ if(prc(pr)) printf("MAX_SIM_DEPTH   : %s\n",yytext); return MAX_SIM_DEPTH;}
"min_sim_depth"		{ if(prc(pr)) printf("MIN_SIM_DEPTH   : %s\n",yytext); return MIN_SIM_DEPTH;}
"sim_depth_step"	{ if(prc(pr)) printf("SIM_DEPTH_DELTA   : %s\n",yytext); return SIM_DEPTH_DELTA;}
"sim_method_disc"	{ if(prc(pr)) printf("SIM_METHOD_DISC   : %s\n",yytext); return SIM_METHOD_DISC;}
"sim_method_exp"	{ if(prc(pr)) printf("SIM_METHOD_EXP   : %s\n",yytext); return SIM_METHOD_EXP;}
"app_crypt"	{ if(prc(pr)) printf("RNG_APP_CRYPT   : %s\n",yytext); return RNG_APP_CRYPT;}
"prism"		{ if(prc(pr)) printf("RNG_PRISM   : %s\n",yytext); return RNG_PRISM;}
"ciardo"	{ if(prc(pr)) printf("RNG_CIARDO   : %s\n",yytext); return RNG_CIARDO;}
"ymer"		{ if(prc(pr)) printf("RNG_YMER   : %s\n",yytext); return RNG_YMER;}
"gsl_ranlux"	{ if(prc(pr)) printf("RNG_GSL_RANLUX   : %s\n",yytext); return RNG_GSL_RANLUX;}
"gsl_lfg"	{ if(prc(pr)) printf("RNG_GSL_LFG   : %s\n",yytext); return RNG_GSL_LFG;}
"gsl_taus"	{ if(prc(pr)) printf("RNG_GSL_TAUS   : %s\n",yytext); return RNG_GSL_TAUS;}

"on"		{ if(prc(pr)) printf("ON   : %s\n",yytext); return ON;}
"off"		{ if(prc(pr)) printf("OFF   : %s\n",yytext); return OFF;}

"tt"		{ if(prc(pr)) printf("TRUE   : %s\n",yytext); return TTRUE;}
"ff"		{ if(prc(pr)) printf("FALSE   : %s\n",yytext); return FFALSE;}
"S"		{ if(prc(pr)) printf("STEADY_STATE_F   : %s\n",yytext); return STEADY_STATE_F;}
"P"		{ if(prc(pr)) printf("PROBABILITY_F   : %s\n",yytext); return PROBABILITY_F;}
"L"		{ if(prc(pr)) printf("LONG_RUN_F   : %s\n",yytext); return LONG_RUN_F;}
"E"		{ if(prc(pr)) printf("EXPECTED_REWARD_RATE_F   : %s\n",yytext); return EXPECTED_REWARD_RATE_F;}
"C"		{ if(prc(pr)) printf("INSTANTANEOUS_REWARD_F   : %s\n",yytext); return INSTANTANEOUS_REWARD_F;}
"Y"		{ if(prc(pr)) printf("EXPECTED_ACCUMULATED_REWARD_F   : %s\n",yytext); return EXPECTED_ACCUMULATED_REWARD_F;}
"X"		{ if(prc(pr)) printf("NEXT_F   : %s\n",yytext); return NEXT_F;}
"U"		{ if(prc(pr)) printf("UNTIL_F   : %s\n",yytext); return UNTIL_F;}

"$RESULT"	{ if(prc(pr)) printf("RESULT   : %s\n",yytext); return RESULT;}
"$STATE"	{ if(prc(pr)) printf("STATE   : %s\n",yytext); return STATE;}
{let}{alnum}*   { if(prc(pr)) printf("ATOMIC_PROPOSITION : %s %d\n",yytext, ATOMIC_PROPOSITION); yylval.sval=yytext; return ATOMIC_PROPOSITION;}

"\n"		{ if(prc(pr)) printf("NEWLINE\n"); return NEWLINE;}
"\r"		{ if(prc(pr)) printf("NEWLINE\n"); return NEWLINE;}
"\r\n"		{ if(prc(pr)) printf("NEWLINE\n"); return NEWLINE;}
{white}	    	{ if(prc(pr)) printf(" ");}
%%
