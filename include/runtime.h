/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: runtime.h,v $
*	$Revision: 1.47 $
*	$Date: 2007/10/14 15:11:57 $
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
*	Source description: Store global variable: state_space, labelling, result_bitset.
*	Uses: DEF: sparse.h, bitset.h, label.h, runtime.h
*		LIB: sparse.c, bitset.c, label.c, runtime.c
*		Definition of runtime - runtime.h
*/

#include "macro.h"

#include "bitset.h"
#include "partition.h"
#include "label.h"
#include "sparse.h"
//Is needed for the runtime.h
#include "simulation.h"


#ifndef RUNTIME_H

#define RUNTIME_H

//The runtime flag settings
#define BLANK_MODE		0x00000000
/*This one is used to test whether a logic has been set.
*I.e. ANY_LOGIC_MODE == CSL_MODE | PCTL_MODE | PRCTL_MODE | CSRL_MODE */
#define ANY_LOGIC_MODE		0x0000000F	// 00000000 00001111
#define CSL_MODE		0x00000001	// 00000000 00000001
#define PCTL_MODE		0x00000002	// 00000000 00000010
#define PRCTL_MODE		0x00000004	// 00000000 00000100
#define CSRL_MODE		0x00000008	// 00000000 00001000
#define F_IND_LUMP_MODE		0x00000010	// 00000000 00010000
#define F_DEP_LUMP_MODE		0x00000020	// 00000000 00100000
#define SIMULATION_MODE		0x00000040	// 00000000 01000000
#define TEST_VMV_MODE		0x80000000	// ?????????????????

//ToDo: Make this in one bitset, all of them are just flags
#define GJ 11  /*Gaus-Jacobi*/
#define GS 12  /*Gaus-Seidel*/
#define US 13  /* uniformization Sericola */
#define UQS 14 /* uniformization Qureshi & Sanders */
#define DTV 15 /* discretization Tijms & Veldman */
#define REC 16 /* recursive version of BSCC search */
#define NON_REC 17 /* non-recursive version of BSCC search */

/************************************************************************************/
/******************************THE RUN-MODE ACCESS FUNCTIONS*************************/
/************************************************************************************/

/**
* This method sets the mode in which we run the mcc tool
* It sets a flag to a bitset which can have the following flags
* on or off:  CSL_MODE, PCTL_MODE, PRCTL_MODE etc.
*/
extern void addRunMode(unsigned int flag);

/**
* This method clears part of the mode in which we run the mcc tool
* It clears the flag from a bitset which can have the following flags
* on or off:  CSL_MODE, PCTL_MODE, PRCTL_MODE etc.
*/
extern void clearRunMode(unsigned int flag);

/**
* This method tests the mode in which we run the mcc tool
* It returns true(The logic ID) if the given flag is set
* in a mode bitset which can have the following flags on
* or off: CSL_MODE, PCTL_MODE, PRCTL_MODE etc.
*
* Example: CSL_MODE == isRunMode(CSL_MODE) in case we are
* in CSL_MODE otherwise BLANK_MODE == isRunMode(CSL_MODE)
*/
extern unsigned int isRunMode(unsigned int flag);

/**
* This method using the mode local file variable and the ANY_LOGIC_MODE
* macros value tests whether the logic has been alreadu set. Returnes
* True if yes, otherwise, False.
*/
extern unsigned int isLogicRunModeSet();

/************************************************************************************/
/****************************THE FORMULA-TREE ACCESS FUNCTIONS***********************/
/************************************************************************************/

/**
* Sets the formula tree after it has been modelchecked
* @param void* the formula tree with the results.
*/
extern void set_formula_tree_result(void *);

/**
* Gets the formula tree after it has been modelchecked
* @return the formula tree with the results.
*/
extern void * get_formula_tree_result();

/************************************************************************************/
/****************************THE STATE-SPACE ACCESS FUNCTIONS************************/
/************************************************************************************/

/*****************************************************************************
name		: set_state_space
role       	: set state space for global access using get_state_space.
@param     	: sparse *: the state space.
remark 	        :
******************************************************************************/
extern void set_state_space(sparse *);

/*****************************************************************************
name		: get_state_space
role       	: globally access state space.
@return		: sparse *: returns a pointer to a sparse matrix (the state space).
remark 	        :
******************************************************************************/
extern sparse * get_state_space();

/************************************************************************************/
/*****************************THE LABELLING ACCESS FUNCTIONS*************************/
/************************************************************************************/

/**
* Sets labelling function for global access using get_labeller.
* @param labelling the labelling function.
*/
extern void set_labeller(labelling *);

/**
* Access the labelling function.
* @return returns a pointer to the structure labelling.
*/
extern labelling *get_labeller();

/************************************************************************************/
/***************************THE STATE REWARDS ACCESS FUNCTIONS***********************/
/************************************************************************************/

/**
* Set State Rewards
*/
extern void setStateRewards(double * _pRewards);

/**
* Get State Rewards
*/
extern double * getStateRewards();

/**
* Free memory
*/
extern void freeStateRewards();

/************************************************************************************/
/**************************IMPULSE REWARDS ACCESS FUNCTIONS**************************/
/************************************************************************************/

/**
* Get the value of impulse_rewards
* @return impulse rewards
*/
extern sparse * getImpulseRewards();

/**
* Set the value of pImpulseRewards
* @param sparse* _impulse_rewards
*/
extern void setImpulseRewards(sparse *);

/**
* This method is used to free the impulse reward structure.
*/
extern void freeImpulseRewards();

/************************************************************************************/
/********************************SIMULATION MODE SETTINGS****************************/
/************************************************************************************/

/**
* This method should be used to set the simulation ON/OFF
* indicator in the runtime settings.
* @param BOOL val TRUE to turn simulation ON.
* Provide FALSE for switching it OFF.
*/
extern void setSimulationStatus(BOOL val);

/**
* This method should be used to get the simulation ON/OFF
* indicator in the runtime settings.
* @return TRUE if the simulation engine is ON, otherwise FALSE.
*/
extern BOOL isSimulationOn();

/**
* This method allows to set the initial state for the "one initial state" simulation.
* NOTE: We do not call the setSimInitialState(int) method of simulation.h directly
*	because if case of formula independent lumping the state-space is partitioned.
* @param initial_state the initial state index
*/
inline setSimInitialStateRuntime( const int user_initial_state );

/************************************************************************************/
/*********************************LUMPING MODE SETTINGS******************************/
/************************************************************************************/

/**
* This method is used for setting the partitioning
* for formula independent lumping
* @param _P the partition for formula independent lumping
*/
extern void setPartition(partition *_P);

/**
* This method is used for getting the partitioning
* for formula independent lumping
* @return The partition that was set before by the
*	  setPartition(partition *) method or NULL.
*/
extern partition * getPartition();

/**
* This method is used for freeying the memory of partition
* for formula independent lumping
*/
extern void freePartition();

/**
* Lets us know whether we are working with the statespace after it was lumped.
* @return TRUE if we are working with the lumped markov chain
*/
extern BOOL isFormulaLumpingDone();

/**
* Is used to set Formula dependent lumping ON/OFF
* @param _formula_lumping_is_done TRUE if we just did the formula dependent lumping
*					  FALSE if we unlumped the state space etc. back.
*/
extern void setFormulaLumpingDone(BOOL _formula_lumping_is_done);

/**
* Here we get the original state-space size,
* because there could be lumping done before.
* @return the original state-space size
*/
extern int get_original_state_space_size();

/************************************************************************************/
/****************************MINOR SETTING ACCESS FUNCTIONS**************************/
/************************************************************************************/

/**
* Gets the value of w
*/
extern double get_w();

/**
* Sets the value of w
* @param double the value of w
*/
extern void set_w(double);

/*****************************************************************************
name		: get_d_factor
role       	: get the value of d
@param     	:
remark 	        : double: d
******************************************************************************/
extern double get_d_factor();

/*****************************************************************************
name		: set_d_factor
role       	: set the value of d_factor
@param     	: double: _d_factor
remark 	        :
******************************************************************************/
extern void set_d_factor(double _d_factor);

/*****************************************************************************
name		: get_max_iterations
role       	: get the value of max_iterations
@param     	:
remark 	        : int: max_iterations
******************************************************************************/
extern int get_max_iterations();

/*****************************************************************************
name		: set_max_iterations
role       	: set the value of max_iterations
@param     	: int: max_iterations
remark 	        :
******************************************************************************/
extern void set_max_iterations(int);

/*****************************************************************************
name		: get_underflow
role       	: get the value of underflow
@param     	:
remark 	        : double: underflow
******************************************************************************/
extern double get_underflow();

/*****************************************************************************
name		: set_underflow
role       	: set the value of underflow
@param     	: double: underflow
remark 	        :
******************************************************************************/
extern void set_underflow(double underflow);

/*****************************************************************************
name		: get_overflow
role       	: get overflow
@param     	:
remark 	        : double: overflow
******************************************************************************/
extern double get_overflow();

/*****************************************************************************
name		: set_overflow
role       	: set the value of overflow
@param     	: double: overflow
remark 	        :
******************************************************************************/
extern void set_underflow(double overflow);

/*****************************************************************************
name		: set_method_path
role       	: set method_path for global access.
@param     	: double: method_path
remark 	        : should be either GJ or GS
******************************************************************************/
extern void set_method_path(int);

/*****************************************************************************
name		: get_method_path
role       	: get method_path
@param     	:
remark 	        : double: method_path
******************************************************************************/
extern int get_method_path();

/*****************************************************************************
name		: set_method_steady
role       	: set method_steady for global access.
@param     	: double: method_steady
remark 	        : should be either GJ or GS
******************************************************************************/
extern void set_method_steady(int _method_steady);

/*****************************************************************************
name		: get_method_steady
role       	: get method_steady
@param     	:
remark 	        : int: method_steady
******************************************************************************/
extern int get_method_steady();

/*****************************************************************************
name		: set_method_until_rewards
role       	: set the method for the evaluation of time-reward-bounded until.
@param     	: int: the method.
remark 	        :
******************************************************************************/
extern void set_method_until_rewards(int);

/*****************************************************************************
name		: get_method_until_rewards
role       	: globally access the result bitset
@return		: int: the method.
remark 	        :
******************************************************************************/
extern int get_method_until_rewards();

/*****************************************************************************
name		: set_error_bound
role       	: set error_bound for global access.
@param     	: double: the error bound
remark 	        :
******************************************************************************/
extern void set_error_bound(double);

/*****************************************************************************
name		: get_error_bound
role       	: get error_bound.
@param     	:
remark 	        : double: the error bound
******************************************************************************/
extern double get_error_bound();

/************************************************************************************/
/******************************THE BSCC SEARCH SETTINGS******************************/
/************************************************************************************/

/**
* Set method for the BSCC search
* @param the method to be set
* NOTE: The method should be either REC or NON_REC
*/
extern void set_method_bscc(int);

/**
* Get method for the BSCC search
* @param the method to be set
* NOTE: The method should be either REC or NON_REC
*/
extern int get_method_bscc();

/************************************************************************************/
/***************************THE STEADY-STATE DETECTION SETTINGS**********************/
/************************************************************************************/

/**
* Enables steady-state detection for uniformization (CSL)
*/
void set_ssd_on();

/**
* Disables steady-state detection for uniformization (CSL)
*/
extern void set_ssd_off();

/**
* @return TRUE if steady-state detection is enabled, otherwise FALSE
*/
extern BOOL is_ssd_on();

/************************************************************************************/
/*********************************THE TECHNICAL FUNCTIONS****************************/
/************************************************************************************/

/*****************************************************************************
name		: get_row_sums
role		: globally access row sums.
		This array contains the sum of ALL row elements of the current
		state_space martix. It means that including the diagonal values and this it is not exacrly what you have on the diagonal of the Generator matrix! To obtain the i'th diagonal value of the generator matrix you should do
		(state_space->val[i].diag - row_sums[i])
@return	: double *:2 the row sums.
remark		:
******************************************************************************/
extern double * get_row_sums();

/*****************************************************************************
name		: free_row_sums
role		: free the row_sums array.
remark		: Should be done along with freying or resetting the
		state_space variable in the runtime.c
******************************************************************************/
extern void free_row_sums();

/************************************************************************************/
/*********************************PRINTING MODE SETTINGS*****************************/
/************************************************************************************/

/**
* This method should be used to set the printing ON/OFF
* indicator in the runtime settings.
* @param BOOL val TRUE to turn printing of the
*				  probabilities and states ON.
*				  Provide FALSE for switching
*				  it OFF.
*/
extern void setPrintingStatus(BOOL val);

/**
* This method should be used to get the printing ON/OFF
* indicator in the runtime settings.
* @return TRUE if the printing of the probabilities and
*	    states is ON, otherwise FALSE.
*/
extern BOOL isPrintingOn();

/************************************************************************************/
/**********************************THE PRINTING FUNCTIONS****************************/
/************************************************************************************/

/**
* This method is used for printing the logic name and also extra options such as Lumping mode.
* @param logic: if set to TRUE then the Logic info is printed
* @param extra: if set to TRUE then the Lumping info is printed
*/
extern inline void print_run_mode(BOOL logic, BOOL extra);

/**
* This method is used to print the runtime settings of error
* bounds, maximum number of iterations etc.
*/
extern inline void print_runtime_info();

/**
* This method is used for printing a bitset which is an
* indicator set of states that satisfy the property
* @param b the bitset which has to be printed
* @param name the bitset name string
*/
extern void print_states(bitset *b, const char * name);

/**
* This method is used for printing TRUE/FALSE result for a state index.
* TRUE if the state 'index' is in the set, otherwise FALSE.
* This method handles lumping.
* @param pBitset: the bitset which has to be printed
* @param index: the state index in the original state space.
* @param name the bitset name string
*/
extern void print_state( const bitset * pBitset, const int index, const char * name );

/**
* This method is used for printing the state probabilities
* Depending whether there was lumping done or not to the
* state space it does different printing
* @param size the size of the probabilities vector
* @param probs the vector of probabilities
* @param name the bitset name string
*/
extern void print_state_probs(int size, double * probs, const char * name );

/**
* This function prints the probability for the state with the given index.
* Possible lumping is taken into account.
* @param index: the original-state-space index (even if lumping was done)
* @param size: the size of 'probs' array
* @param probs: the array of probabilities
* @param name the bitset name string
*/
extern inline void print_state_prob( const int index, const int size, const double * probs, const char * name);

/**
* This method is used for getting the printing pattern from the error bound.
* NOTE: If we have error 1.231e-10 then the presision
* is 10+1 digits after the decimal point, but not 13!
* @return the precision for the printing pattern by taking the position
*	    of the first significant digit after the decimal point + 1
*/
extern inline int get_error_bound_precision();

/**
* This method is used for getting the printing pattern from the value.
* NOTE: If we have a value 1.231e-10 then the presision
* is 10+1 digits after the decimal point, but not 13!
* @param value: the value to evaluate
* @return the precision for the printing pattern by taking the position
*	    of the first significant digit after the decimal point + 1
*/
extern inline int get_precision(double value);

#endif
