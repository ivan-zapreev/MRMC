/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: runtime.c,v $
*	$Revision: 1.80 $
*	$Date: 2007/10/14 15:24:38 $
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

#include "runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <math.h>

/**
* This boolean variable indicates the formula independent lumping is done already.
* It should allow to avoid cases when we have hested call of operators each of
* which wants to do lumping.
*/
static BOOL formula_lumping_is_done = FALSE;

/**
* This will be used as a bitset of 32 bits to hold the input
* and runtime flag settings, such as: CSL_MODE, ... , F_DEP_LUMPING_MODE,
* TEST_VMV_MODE or combinations of them.
*/
static unsigned int mode = BLANK_MODE;

/**
* True if the steady-state detection is on
*/
static BOOL ssd_on = FALSE;

/**
* Holds the curent statespace, it is changed if lumping is used.
*/
static sparse *state_space = NULL;

/**
* The partitioning for formula independent lumping
*/
static partition *P = NULL;

/**
* Allowed to be NULL, if not specified, for MRMs (continuous).
*/
static sparse *pImpulseRewards = NULL;

static labelling *labeller = NULL;

/**
* This array contains the sum of ALL row elements of the current
* state_space martix. It means that including the diagonal values
* and this it is not exacrly what you have on the diagonal of the
* Generator matrix! To obtain the i'th diagonal value of the
* generator matrix you should do (state_space->val[i].diag - row_sums[i])
*/
static double *row_sums = NULL; 

/**
* Rewards for DTRM
*/
static double * pRewards = NULL;
static double error_bound=1e-6;
static int method_path=GS, method_steady=GS;
static int method_bscc=REC;
static int method_until_rewards=DTV;
static int max_iterations=1000000;
static double un=DBL_MIN, ov=DBL_MAX;
static double d_factor = (double)1/32.0;
static double w = 1e-11;

/************************************************************************************/
/******************************THE RUN-MODE ACCESS FUNCTIONS*************************/
/************************************************************************************/

/**
* This method sets the mode in which we runthe mcc tool
* It sets a flag to a bitset which can have the following flags
* on or off:  CSL_MODE, PCTL_MODE, PRCTL_MODE etc.
*/
void addRunMode(unsigned int flag)
{
	set_flag(mode, flag);
}

/**
* This method clears part of the mode in which we runthe mcc tool
* It clears the flag from a bitset which can have the following flags
* on or off:  CSL_MODE, PCTL_MODE, PRCTL_MODE etc.
*/
void clearRunMode(unsigned int flag)
{
	clear_flag(mode, flag);
}

/**
* This method tests the mode in which we runthe mcc tool
* It returns true(The logic ID) if the given flag is set
* in a mode bitset which can have the following flags on
* or off: CSL_MODE, PCTL_MODE, PRCTL_MODE etc.
*
* Example: CSL_MODE == isRunMode(CSL_MODE) in case we are
* in CSL_MODE otherwise BLANK_MODE == isRunMode(CSL_MODE)
*/
unsigned int isRunMode(unsigned int flag)
{
	return test_flag(mode, flag);
}

/**
* This method using the mode local file variable and the ANY_LOGIC_MODE
* macros value tests whether the logic has been alreadu set. Returnes
* True if yes, otherwise, False.
*/
unsigned int isLogicRunModeSet()
{
	return test_flag(mode, ANY_LOGIC_MODE);
}

/************************************************************************************/
/****************************THE FORMULA-TREE ACCESS FUNCTIONS***********************/
/************************************************************************************/

//This variable stores the globally accessible result of the
//lately model-checked formula. It is in a form of a formulatree
static void* pFormulaTreeRootNode = NULL;

/**
* Sets the formula tree after it has been modelchecked
* @param pFormulaTreeRootNode the formula tree with the results.
*/
void set_formula_tree_result(void * _pFormulaTreeRootNode){
	pFormulaTreeRootNode = _pFormulaTreeRootNode;
}

/**
* Gets the formula tree after it has been modelchecked
* @return the formula tree with the results.
*/
void * get_formula_tree_result(){
	return pFormulaTreeRootNode;
}

/************************************************************************************/
/****************************THE STATE-SPACE ACCESS FUNCTIONS************************/
/************************************************************************************/

/*****************************************************************************
name		: set_state_space
role		: set state space for global access using get_state_space.
@param		: sparse *: the state space.
remark		:
******************************************************************************/
void set_state_space(sparse *space)
{
	state_space = space;
	set_sparse(state_space);
	row_sums = get_mtx_row_sums(space);
}

/*****************************************************************************
name		: get_state_space
role		: globally access state space.
@return		: sparse *: returns a pointer to a sparse matrix (the state space).
remark 	:
******************************************************************************/
sparse * get_state_space()
{
	return state_space;
}

/************************************************************************************/
/*****************************THE LABELLING ACCESS FUNCTIONS*************************/
/************************************************************************************/

/**
* Sets labelling function for global access using get_labeller.
* @param labelling the labelling function.
*/
void set_labeller(labelling *labellin)
{
	labeller = labellin;
}

/**
* Access the labelling function.
* @return returns a pointer to the structure labelling.
*/
labelling *get_labeller()
{
	return labeller;
}

/************************************************************************************/
/***************************THE STATE REWARDS ACCESS FUNCTIONS***********************/
/************************************************************************************/

/**
* Set State Rewards
*/
void setStateRewards(double * _pRewards)
{
	pRewards = _pRewards;
}

/**
* Get Rewards
*/
double * getStateRewards()
{
	return pRewards;
}

/**
* Free memory
*/
void freeStateRewards()
{
	if( pRewards ) free(pRewards);
	pRewards = NULL;
}

/************************************************************************************/
/**************************IMPULSE REWARDS ACCESS FUNCTIONS**************************/
/************************************************************************************/

/**
* Get the value of impulse_rewards
* @return impulse rewards
*/
sparse * getImpulseRewards()
{
	return pImpulseRewards;
}

/**
* Set the value of pImpulseRewards
* @param sparse* _impulse_rewards
*/
void setImpulseRewards(sparse * _pImpulseRewards)
{
	pImpulseRewards=_pImpulseRewards;
	set_method_until_rewards(UQS);
}

/**
* This method is used to free the impulse reward structure.
*/
void freeImpulseRewards(){
	if(pImpulseRewards){
		free_sparse_ncolse(pImpulseRewards);
		pImpulseRewards = NULL;
	}
}

/************************************************************************************/
/********************************SIMULATION MODE SETTINGS****************************/
/************************************************************************************/

/**
* This method should be used to set the simulation ON/OFF
* indicator in the runtime settings.
* @param BOOL val TRUE to turn simulation ON.
* Provide FALSE for switching it OFF.
* NOTE: in case the run mode is F_DEP_LUMP_MODE the simulation mode does not work
*/
void setSimulationStatus(BOOL val){
	if( val ){
		if( ! isRunMode(F_DEP_LUMP_MODE) && ! isRunMode(F_IND_LUMP_MODE) &&
			( isRunMode(CSL_MODE) || isRunMode(CSRL_MODE) ) ){
			//Update the runtime mode
			addRunMode( SIMULATION_MODE );
			
			//The default RNG methods are set beforehand, but GSL functions
			//need an explicit initialization.
			//Generating of the new seeds is done in the set methods
			setRNGMethodDiscrete( getRNGMethodDiscrete() );
			setRNGMethodExp( getRNGMethodExp() );
		}else{
			printf("WARNING: The simulation engine is only available in CSL (CSRL) mode without lumping.\n");
			printf("WARNING: The 'set' command is ignored.\n");
		}
	}else{
		//Update the runtime mode
		clearRunMode( SIMULATION_MODE );
		
		//Free the random-number generator data, especially needed by GSL functions
		freeRNGDiscrete();
		freeRNGExp();
	}
}

/**
* This method should be used to get the simulation ON/OFF
* indicator in the runtime settings.
* @return TRUE if the simulation engine is ON, otherwise FALSE.
*/
BOOL isSimulationOn(){
	return (isRunMode( SIMULATION_MODE ) ? TRUE : FALSE);
}

/**
* This method allows to set the initial state for the "one initial state" simulation.
* NOTE: We do not call the setSimInitialState(int) method of simulation.h directly
*	because if case of formula independent lumping the state-space is partitioned.
* @param initial_state the initial state index
*/
inline setSimInitialStateRuntime( const int user_initial_state ){
	int true_initial_state = user_initial_state - 1;
	if( isRunMode(F_IND_LUMP_MODE) ){
		true_initial_state = getLumpedStateIndex( getPartition(), true_initial_state );
	}
	//Check if the index is within the allowed range, not that for the number
	//of state in the original state space is allways not smaller than in the
	//lumped one. Therefore this check does no harm in any case.
	if( ( true_initial_state >= 0 ) && ( true_initial_state < get_state_space()->rows ) ){
		setSimInitialState( true_initial_state );
	} else {
		printf("ERROR: The state index is outside the allowed state range.\n");
		printf("WARNING: The 'set' command is ignored.\n");
	}
}

/************************************************************************************/
/*********************************LUMPING MODE SETTINGS******************************/
/************************************************************************************/

/**
* This method is used for setting the partitioning
* for formula independent lumping
* @param _P the partition for formula independent lumping
*/
void setPartition(partition *_P){
	P = _P;
}

/**
* This method is used for getting the partitioning
* for formula independent lumping
* @return The partition that was set before by the
*	  setPartition(partition *) method or NULL.
*/
partition * getPartition(){
	return P;
}

/**
* This method is used for freeying the memory of partition
* for formula independent lumping
*/
void freePartition(){
	if( isRunMode(F_IND_LUMP_MODE) && P){
		free_partition(P);
		P = NULL;
	}
}

/**
* Lets us know whether we are working with the statespace after it was lumped.
* @return TRUE if we are working with the lumped markov chain
*/
BOOL isFormulaLumpingDone(){
	return formula_lumping_is_done;
}

/**
* Is used to set Formula dependent lumping ON/OFF
* @param _formula_lumping_is_done TRUE if we just did the formula dependent lumping
*					  FALSE if we unlumped the state space etc. back.
*/
void setFormulaLumpingDone(BOOL _formula_lumping_is_done){
	if( formula_lumping_is_done && _formula_lumping_is_done ){
		printf("ERROR: Trying to do second formula dependent lumping in a row!\n");
	}
	formula_lumping_is_done = _formula_lumping_is_done;
}

/**
* Here we get the original state-space size,
* because there could be lumping done before.
* @return the original state-space size
*/
inline int get_original_state_space_size(){
	int result = 0;
	partition *P = getPartition();
	if(isRunMode(F_IND_LUMP_MODE) && P != NULL){
		result = get_unlumped_state_space_size(P);
	}else{
		result = get_labeller()->ns;
	}
	return result;
}

/************************************************************************************/
/****************************MINOR SETTING ACCESS FUNCTIONS**************************/
/************************************************************************************/

/**
* Gets the value of w
*/
double get_w()
{
	return w;
}

/**
* Sets the value of w
* @param double the value of w
*/
void set_w(double _w)
{
	w=_w;
}


/*****************************************************************************
name		: get_d_factor
role       	: get the value of d
@param     	: 
remark 	        : double: d
******************************************************************************/
double get_d_factor()
{
	return d_factor;
}

/*****************************************************************************
name		: set_d_factor
role       	: set the value of d_factor
@param     	: double: _d_factor
remark 	        : 
******************************************************************************/
void set_d_factor(double _d_factor)
{
	d_factor=_d_factor;
}

/*****************************************************************************
name		: get_max_iterations
role       	: get the value of max_iterations
@param     	: 
remark 	        : int: max_iterations
******************************************************************************/
int get_max_iterations()
{
	return max_iterations;
}

/*****************************************************************************
name		: set_max_iterations
role       	: set the value of max_iterations
@param     	: int: max_iterations
remark 	        : 
******************************************************************************/
void set_max_iterations(int _max_iterations)
{
	max_iterations=_max_iterations;
}

/*****************************************************************************
name		: get_underflow
role       	: get the value of underflow
@param     	: 
remark 	        : double: underflow
******************************************************************************/
double get_underflow()
{
	return un;
}

/*****************************************************************************
name		: set_underflow
role       	: set the value of underflow
@param     	: double: underflow
remark 	        : 
******************************************************************************/
void set_underflow(double underflow)
{
	un=underflow;
}

/*****************************************************************************
name		: get_overflow
role       	: get overflow
@param     	: 
remark 	        : double: overflow
******************************************************************************/
double get_overflow()
{
	return ov;
}

/*****************************************************************************
name		: set_overflow
role       	: set the value of overflow
@param     	: double: overflow
remark 	        : 
******************************************************************************/
void set_overflow(double overflow)
{
	ov=overflow;
}

/*****************************************************************************
name		: set_method_path
role       	: set method_path for global access.
@param     	: double: method_path
remark 	        : should be either GJ or GS
******************************************************************************/
void set_method_path(int _method_path)
{
	method_path = _method_path;
}

/*****************************************************************************
name		: get_method_path
role       	: get method_path
@param     	: 
remark 	        : double: method_path
******************************************************************************/
int get_method_path()
{
	return method_path;
}

/*****************************************************************************
name		: set_method_steady
role       	: set method_steady for global access.
@param     	: double: method_steady
remark 	        : should be either GJ or GS
******************************************************************************/
void set_method_steady(int _method_steady)
{
	method_steady = _method_steady;
}

/*****************************************************************************
name		: set_method_until_rewards
role       	: set the method for the evaluation of time-reward-bounded until.
@param     	: int: the method.
remark 	        : 
******************************************************************************/
void set_method_until_rewards(int method_until_rew)
{
	method_until_rewards = method_until_rew;
}

/*****************************************************************************
name		: get_method_until_rewards
role       	: globally access the result bitset
@return		: int: the method.
remark 	        : 
******************************************************************************/
int get_method_until_rewards()
{
	return method_until_rewards;
}

/*****************************************************************************
name		: get_method_steady
role       	: get method_steady
@param     	: 
remark 	        : double: method_steady
******************************************************************************/
int get_method_steady()
{
	return method_steady;
}

/*****************************************************************************
name		: set_error_bound
role       	: set error_bound for global access.
@param     	: double: the error bound
remark 	        : 
******************************************************************************/
void set_error_bound(double _error_bound)
{
	error_bound = _error_bound;
}

/*****************************************************************************
name		: get_error_bound
role       	: get error_bound.
@param     	: 
remark 	        : double: the error bound
******************************************************************************/
double get_error_bound()
{
	return error_bound;
}

/************************************************************************************/
/******************************THE BSCC SEARCH SETTINGS******************************/
/************************************************************************************/

/**
* Set method for the BSCC search
* @param the method to be set
* NOTE: The method should be either REC or NON_REC
*/
void set_method_bscc(int _method_bscc)
{
	method_bscc = _method_bscc;
}

/**
* Get method for the BSCC search
* @param the method to be set
* NOTE: The method should be either REC or NON_REC
*/
int get_method_bscc()
{
	return method_bscc;
}

/************************************************************************************/
/***************************THE STEADY-STATE DETECTION SETTINGS**********************/
/************************************************************************************/

/**
* This method is used to set the steady-state detection on and off
* @param _on_off TRUE for setting the steady-state detection on, otherwise FALSE.
*/
void set_ssd(BOOL _on_off)
{
  ssd_on = _on_off;
}

/**
* Enables steady-state detection for uniformization (CSL)
*/
void set_ssd_on()
{
  set_ssd(TRUE);
}

/**
* Disables steady-state detection for uniformization (CSL)
*/
void set_ssd_off()
{
  set_ssd(FALSE);
}

/**
* @return TRUE if steady-state detection is enabled, otherwise FALSE
*/
BOOL is_ssd_on()
{
   return ssd_on;
}

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
double * get_row_sums()
{
	return row_sums;
}


/*****************************************************************************
name		: free_row_sums
role		: free the row_sums array.
remark		: Should be done along with freying or resetting the
		state_space variable from the runtime.c
******************************************************************************/
void free_row_sums()
{
	free(row_sums);
	row_sums = NULL;
}

/************************************************************************************/
/*********************************PRINTING MODE SETTINGS*****************************/
/************************************************************************************/

static BOOL printing_status = TRUE;

/**
* This method should be used to set the printing ON/OFF
* indicator in the runtime settings.
* Prints a warning message if the status is set to FALSE
* @param BOOL val TRUE to turn printing of the
*				  probabilities and states ON.
*				  Provide FALSE for switching
*				  it OFF.
*/
void setPrintingStatus(BOOL val){
	printing_status = val;
	if( ! val ){
		printf("WARNING: Printing of results is now mostly disabled, use 'help' for more options.\n");
	}
}

/**
* This method should be used to get the printing ON/OFF
* indicator in the runtime settings.
* @return TRUE if the printing of the probabilities and
*	    states is ON, otherwise FALSE.
*/
BOOL isPrintingOn(){
	return printing_status;
}

/************************************************************************************/
/**********************************THE PRINTING FUNCTIONS****************************/
/************************************************************************************/

/**
* This method is used for printing the logic name and also extra options such as Lumping mode.
* @param logic: if set to TRUE then the Logic info is printed
* @param extra: if set to TRUE then the Lumping info is printed
*/
inline void print_run_mode(BOOL logic, BOOL extra){
	if( logic ){
		//Logic mode
		if( isRunMode(TEST_VMV_MODE) ){
			printf(" Mode\t\t\t = SELF TEST\n");
		}else{
			printf(" Logic\t\t\t = ");
			if( isRunMode(CSL_MODE) ){
				printf("CSL\n");
			}else{
				if( isRunMode(PCTL_MODE) ){
					printf("PCTL\n");
				}else{
					if( isRunMode(PRCTL_MODE) ){
						printf("PRCTL\n");
					}else{
						if( isRunMode(CSRL_MODE) ){
							printf("CSRL\n");
						}else{
							printf("UNKNOWN!\n");
						}
					}
				}
			}
		}
	}
	if( extra ){
		printf(" Formula ind. lumping\t = %s\n",(isRunMode(F_IND_LUMP_MODE) ? "ON":"OFF"));
		printf(" Formula dep. lumping\t = %s\n",(isRunMode(F_DEP_LUMP_MODE) ? "ON":"OFF"));
	}
}

/**
* This method is used to print the runtime settings of error
* bounds, maximum number of iterations etc.
*/
inline void print_runtime_info(){
	int mp  = get_method_path();
	int ms  = get_method_steady();
	int mur = get_method_until_rewards();
	int mb  = get_method_bscc();
	
	printf(" ---General settings:\n");
	print_run_mode( TRUE, FALSE );
	print_run_mode( FALSE, TRUE );
	printf(" M. C. simulation\t = %s\n", (isSimulationOn() ? "ON":"OFF" ) );
	if( isRunMode(CSL_MODE) || isRunMode(CSRL_MODE) ){
		printf(" Steady-state detection\t = %s\n", (is_ssd_on() ? "ON":"OFF"));
	}
	printf(" Method Path\t\t = ");
	switch(mp){
		case GJ:
			printf("Gauss-Jacobi\n");
			break;
		case GS:
			printf("Gauss-Seidel\n");
			break;
	}
	printf(" Method Steady\t\t = ");
	switch(ms){
		case GJ:
			printf("Gauss-Jacobi\n");
			break;
		case GS:
			printf("Gauss-Seidel\n");
			break;
	}
	if( isRunMode(CSRL_MODE) ){
		printf(" Method Until Rewards\t = ");
		switch(mur){
			case US:
				printf("Uniformization-Sericola\n");
				break;
			case UQS:
				printf("Uniformization-Qureshi-Sanders\n");
				break;
			case DTV:
				printf("Discretization-Tijms-Veldman\n");
				break;
		}
	}
	printf(" Method BSCC\t\t = ");
	switch(mb){
		case REC:
			printf("Recursive\n");
			break;
		case NON_REC:
			printf("Non-Recursive\n");
			break;
	}
	printf(" Results printing\t = %s\n", (isPrintingOn()? "ON":"OFF") );
	printf("\n");
	
	//Print the simulation runtime parameters
	printRuntimeSimInfo( isRunMode( CSL_MODE ) || isRunMode( CSRL_MODE ) );
	
	printf(" ---Numerical methods:\n");
	printf(" -Iterative solvers:\n");
	printf("   Error Bound\t\t\t = %e\n", get_error_bound());
	printf("   Max Iterations\t\t = %ld\n", get_max_iterations());
	if( isRunMode(CSL_MODE) || isRunMode(CSRL_MODE) ){
		printf(" -Fox-Glynn algorithm:\n");
		printf("   Overflow\t\t\t = %e\n", get_overflow());
		printf("   Underflow\t\t\t = %e\n", get_underflow());
	}
	if( isRunMode(CSRL_MODE) ){
		printf(" -Uniformization Qureshi-Sanders:\n");
		printf("   Probability threshold\t = %e\n", get_w());
		printf(" -Discretization Tijms-Veldman:\n");
		printf("   Discretization factor\t = %e\n", get_d_factor());
	}
}

/**
* This method is used for getting the printing pattern from the error bound.
* NOTE: If we have error 1.231e-10 then the presision
* is 10+1 digits after the decimal point, but not 13!
* @return the precision for the printing pattern by taking the position
*	    of the first significant digit after the decimal point + 1
*/
inline int get_error_bound_precision(){
	return get_precision( get_error_bound() );
}

/**
* This method is used for getting the printing pattern from the value.
* NOTE: If we have a value 1.231e-10 then the presision
* is 10+1 digits after the decimal point, but not 13!
* @param value: the value to evaluate
* @return the precision for the printing pattern by taking the position
*	    of the first significant digit after the decimal point + 1
*/
inline int get_precision(double value){
	int precision = 0;
	BOOL not_found = TRUE;
	double integer = 0, fraction = 0;
	
	//NOTE: If we have error 1.1e-10 then the presision
	//is 10 digits after the decimal point, but not 11.
	while ( not_found ){
		value *= 10;
		fraction = modf(value, &integer);
		if ( integer > 0.0 ){
			not_found = FALSE;
		}
		precision++;
	}
	
	return precision + 1;
}

/**
* This method is used for printing a bitset which is an
* indicator set of states that satisfy the property
* @param b the bitset which has to be printed
* @param name the bitset name string
*/
void print_states(bitset *b, const char * name){
	partition *P = getPartition();
	printf("%s: ", name);
	if( isRunMode(F_IND_LUMP_MODE) && P != NULL ){
		print_original_states(P, b);
	}else{
		print_bitset_states(b);
	}
	printf("\n");
}

/**
* This method is used for printing TRUE/FALSE result for a state index.
* TRUE if the state 'index' is in the set 'pBitset', otherwise FALSE.
* This method handles lumping.
* @param pBitset: the bitset which has to be printed
* @param index: the state index in the original state space.
* @param name the bitset name string
*/
void print_state( const bitset * pBitset, const int index, const char * name ){
	partition *P = getPartition();
	
	printf("%s[%d] = ", name, index+1);
	
	if(isRunMode(F_IND_LUMP_MODE) && P != NULL){
		print_original_state(P, pBitset, index);
	}else{
		if( pBitset != NULL ){
			if( 0 <= index && index < pBitset->n ){
				if( get_bit_val( pBitset, index ) ){
					printf("TRUE");
				}else{
					printf("FALSE");
				}
			}else{
				printf("??\n");
				printf("WARNING: Invalid index %d, required to be in the [1, %d] interval.", index + 1, pBitset->n);
			}
		}else{
			printf("??\n");
			printf("WARNING: Trying to print an element of a non-existing bitset.");
		}
	}
	printf("\n");
}

/**
* This method is used for printing the state probabilities
* Depending whether there was lumping done or not to the
* state space it does different printing
* @param size the size of the probabilities vector
* @param probs the vector of probabilities
* @param name the bitset name string
*/
void print_state_probs(int size, double * probs, const char * name ){
	partition *P = getPartition();
	printf("%s: ", name);
	if( isRunMode(F_IND_LUMP_MODE) && P != NULL ){
		print_state_probs_partition(P, size, probs);
	}else{
		//Calculate the right pattern according to the precision
		char buffer[255];
		sprintf(buffer, "%%1.%dlf", get_error_bound_precision());
		print_pattern_vec_double(buffer, size, probs);
	}
	printf("\n");
}

/**
* This function prints the probability for the state with the given index.
* Possible lumping is taken into account.
* @param index: the original-state-space index (even if lumping was done)
* @param size: the size of 'probs' array
* @param probs: the array of probabilities
* @param name the bitset name string
*/
inline void print_state_prob( const int index, const int size, const double * probs, const char * name){
	partition *P = getPartition();
	printf( "%s[%d] = ", name, index + 1 );
	if( isRunMode(F_IND_LUMP_MODE) && P != NULL ){
		print_state_prob_partition(P, index, size, probs);
	}else{
		if( probs != NULL ){
			if( 0 <= index && index < size){
					//Calculate the right pattern according to the precision
					char buffer[255];
					sprintf(buffer, "%%1.%dlf", get_error_bound_precision());
					printf(buffer, probs[index]);
			}else{
				printf("??\n");
				printf("WARNING: Invalid index %d, required to be in the [1, %d] interval.", index + 1, size);
			}
		}else{
			printf("??\n");
			printf("WARNING: Trying to print an element of a non-existing array.");
		}
	}
	printf("\n");
}
