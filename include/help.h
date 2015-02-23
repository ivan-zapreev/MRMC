/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: help.h,v $
*	$Revision: 1.40 $
*	$Date: 2007/10/14 15:13:43 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
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
*	Source description: This file contains Help Messages.
*/

//The unique identifiers for the help messages type
#define HELP_GENERAL_MSG_TYPE 1
#define HELP_COMMON_MSG_TYPE 2
#define HELP_REWARDS_MSG_TYPE 3
#define HELP_SIMULATION_MSG_TYPE 4
#define HELP_LOGIC_MSG_TYPE 5

#define HELP_GENERAL_MSG " quit\t - exit the program.\n\
 help HT - display a help info on a given topic.\n\
 print\t - print run-time settings.\n\
 print tree - print the formula tree with the results and supplementary information.\n\
 $RESULT[N] - access the computed results of U, X, L, S, E, C, Y operators by a state index.\n\
 $STATE[N]  - access the state-formula satisfiability set by a state index.\n\
 set *\t - Where * is one of the following:\n\
 \t print L\t - Turn on/off most of the resulting output, see '$RESULT[I]' and '$STATE[I]' commands.\n\
 \t simulation L\t - Turn on/off the simulation engine.\n\
 Here:\n\
 \t HT is one of {logic, simulation, rewards, common}.\n\
 \t L is one of {on, off}.\n\
 \t N is a natural number.\n"

#define HELP_COMMON_MSG " set *\t - Where * is one of the following:\n\
 \t ssd L\t\t - Turn on/off the steady-state detection for time bounded until (CTMC model).\n\
 \t error_bound R\t - Error Bound for all iterative methods.\n\
 \t max_iter N\t - Number of Max Iterations for all iterative methods.\n\
 \t overflow R\t - Overflow for the Fox-Glynn algorithm.\n\
 \t underflow R\t - Underflow for the Fox-Glynn algorithm.\n\
 \t method_path M\t - Method for path formulas.\n\
 \t method_steady M - Method for steady state formulas.\n\
 \t method_bscc MB\t - Method for BSCC search.\n\
 Here:\n\
 \t L is one of {on, off}.\n\
 \t R is a real value.\n\
 \t M is one of {gauss_jacobi, gauss_seidel}.\n\
 \t MB is one of {recursive, non_recursive}.\n"

#define HELP_REWARDS_MSG " set *\t - Where * is one of the following:\n\
 \t method_until_rewards MU - Method for time-reward-bounded until formula.\n\
 \t w R\t\t\t - The probability threshold for uniformization Qureshi-Sanders.\n\
 \t d R\t\t\t - The discretization factor for discretization Tijms-Veldman.\n\
 Here:\n\
 \t MU is one of {uniformization_sericola, uniformization_qureshi_sanders, discretization_tijms_veldman}.\n\
 \t R is a real value.\n"

#define HELP_SIMULATION_MSG " set *\t - Where * is one of the following:\n\
 \t sim_type ST\t - Sets the simulation type, i.e. either do simulation for all initial states or just one.\n\
 \t initial_state N\t - Sets the initial state for the simulation type ST == one.\n\
 \t sim_method_steady MS - Sets the simulation mode for the steady-state (long-run) operator.\n\
 \t gen_conf R\t - The confidence level for simulation.\n\
 \t max_sample_size N - The maximum sample size.\n\
 \t min_sample_size N - The minimum sample size.\n\
 \t sample_size_step N - The sample-size increase step.\n\
 \t sim_method_disc RNG - The random-number generator for a discrete distribution.\n\
 \t sim_method_exp RNG - The random-number generator for an exponential distribution (time-interval until, CSL).\n\
 For the simulation of unbounded until and the pure simulation of steady-state (long-run) operator:\n\
 \t conv_conf R\t - The convergence-criteria confidence.\n\
 \t max_sim_depth N - The maximum simulation depth.\n\
 \t min_sim_depth N - The minimum simulation depth.\n\
 \t sim_depth_step N - The simulation-depth increase step.\n\
 Here:\n\
 \t RNG is one of {app_crypt, ciardo, prism, ymer, gsl_ranlux, gsl_lfg, gsl_taus}\n\
 \t ST is one of {one, all}.\n\
 \t MS is one of {pure, hybrid}.\n\
 \t R is a real value.\n\
 \t N is a natural number.\n"

 /**************************/
//DEFINE THE LOGIC FORMULAS//
/**************************/

//"Here:" description common for all logics
#define HELP_LOGIC_COMMON_INFO_MSG " Here:\n\
 \t OP    =   > | < | <= | >=\n\
 \t R is a real value.\n\
 \t N is a natural number.\n"

//The state formulas common for all logics
#define HELP_ALL_LOGIC_COMMON_SF_MSG "\tCONST =   ff | tt\n\
\tSFL   =   CONST\n\
\t\t| LABEL\n\
\t\t| ! SFL\n\
\t\t| SFL && SFL\n\
\t\t| SFL || SFL\n\
\t\t| ( SFL )\n\
\t\t| P{ OP R }[ PFL ]\n"

//The path formulas common for all logics
#define HELP_ALL_LOGIC_COMMON_PF_MSG "\tPFL   =   X SFL\n\
\t\t| SFL U SFL\n"

 /*******************************/
//DEFINE THE PCTL LOGIC FORMULAS//
/*******************************/

//The state formulas common for PCTL
#define HELP_PCTL_LOGIC_COMMON_SF_MSG HELP_ALL_LOGIC_COMMON_SF_MSG "\t\t| L{ OP R }[ SFL ]\n"

//The path formulas common for PCTL
#define HELP_PCTL_LOGIC_COMMON_PF_MSG HELP_ALL_LOGIC_COMMON_PF_MSG "\t\t| SFL U[ N, N ] SFL\n" 

//The formulas common for PCTL
#define HELP_PCTL_MSG " PCTL logic formulas:\n" HELP_PCTL_LOGIC_COMMON_SF_MSG HELP_PCTL_LOGIC_COMMON_PF_MSG HELP_LOGIC_COMMON_INFO_MSG

 /********************************/
//DEFINE THE PRCTL LOGIC FORMULAS//
/********************************/

//The state formulas specific for PRCTL
#define HELP_PRCTL_LOGIC_COMMON_SF_MSG HELP_ALL_LOGIC_COMMON_SF_MSG "\t\t| E [ R, R] [ SFL ] \n\
\t\t| E [N][ R, R] [ SFL ] \n\
\t\t| C [N][ R, R] [ SFL ] \n\
\t\t| Y [N][ R, R] [ SFL ] \n"

//The path formulas common for PRCTL
#define HELP_PRCTL_LOGIC_COMMON_PF_MSG HELP_ALL_LOGIC_COMMON_PF_MSG "\t\t| SFL U[ N, N ][ R, R ] SFL\n"

//The common for PRCTL
#define HELP_PRCTL_MSG " PRCTL logic formulas:\n" HELP_PRCTL_LOGIC_COMMON_SF_MSG HELP_PRCTL_LOGIC_COMMON_PF_MSG HELP_LOGIC_COMMON_INFO_MSG

 /******************************/
//DEFINE THE CSL LOGIC FORMULAS//
/******************************/

//The state formulas common for CSL
#define HELP_CSL_LOGIC_COMMON_SF_MSG HELP_ALL_LOGIC_COMMON_SF_MSG "\t\t| S{ OP R }[ SFL ]\n"

//The path formulas common for CSL
#define HELP_CSL_LOGIC_COMMON_PF_MSG HELP_ALL_LOGIC_COMMON_PF_MSG "\t\t| X[ R, R ] SFL\n\
\t\t| SFL U[ R, R ] SFL\n"

//The common for CSL
#define HELP_CSL_MSG " CSL logic formulas:\n" HELP_CSL_LOGIC_COMMON_SF_MSG HELP_CSL_LOGIC_COMMON_PF_MSG HELP_LOGIC_COMMON_INFO_MSG

 /*******************************/
//DEFINE THE CSRL LOGIC FORMULAS//
/*******************************/

//The state formulas common for CSRL
#define HELP_CSRL_LOGIC_COMMON_SF_MSG HELP_CSL_LOGIC_COMMON_SF_MSG

//The path formulas common for CSRL
#define HELP_CSRL_LOGIC_COMMON_PF_MSG HELP_CSL_LOGIC_COMMON_PF_MSG "\t\t| X [R, R][R, R] SFL\n\
\t\t| SFL U[ R, R ] [ R, R ] SFL\n"

//The common for CSRL
#define HELP_CSRL_MSG " CSRL logic formulas:\n" HELP_CSRL_LOGIC_COMMON_SF_MSG HELP_CSRL_LOGIC_COMMON_PF_MSG HELP_LOGIC_COMMON_INFO_MSG
