/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: mcc.c,v $
*	$Revision: 1.19 $
*	$Date: 2007/10/02 16:10:06 $
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
*	Source description: Contains the main function, starts the execution,
*	does initial validation of input parameters, loads the required input files.
*/

# include "macro.h"

# include <sys/types.h>
# include <sys/time.h>

# include "bitset.h"
# include "sparse.h"
# include "label.h"
# include "runtime.h"
# include "transient.h"
# include "read_rewards.h"
# include "read_tra_file.h"
# include "read_lab_file.h"
# include "read_impulse_rewards.h"
# include "lump.h"
# include "parser_to_core.h"

//This is the list of possible logics
#define CSL_MODE_STR "csl"
#define PCTL_MODE_STR "pctl"
#define PRCTL_MODE_STR "prctl"
#define CSRL_MODE_STR "csrl"

//This is the list of possible options
#define F_IND_LUMP_MODE_STR "-ilump"
#define F_DEP_LUMP_MODE_STR "-flump"

//This "logic" is used for testing vector
//matrix and matrix vector multiplications
#define TEST_VMV_MODE_STR "vmv_test"

//This is the list of possible file extensions
#define TRA_FILE_EXT ".tra"
#define LAB_FILE_EXT ".lab"
#define REW_FILE_EXT ".rew"
#define REWI_FILE_EXT ".rewi"

/**
* An extension can be one of:
* 	.rew, .rewi, .tra, .lab
* plus at least one symbol of the name
*/
#define MIN_FILE_NAME_LENGTH 5
/**
* Here once again the max length of the extentions:
* 	.rew, .rewi, .tra, .lab
* is 5 symbols, this we need to copy the extension out
* of the possible file name.
*/
#define MAX_FILE_EXT_LENGTH 5

//These are the globals used only in this file.
static sparse *space = NULL;		//The statespace
static labelling *labels = NULL;	//The labelling

/**
* This is a list of boolean variables which indicate
* whether certain files were loaded or not
*/
static BOOL is_tra_present  = FALSE;
static BOOL is_lab_present  = FALSE;
static BOOL is_rew_present  = FALSE;
static BOOL is_rewi_present = FALSE;

/**
* Here we will store pointers to the input files
*/
static char* tra_file  = NULL;
static char* lab_file  = NULL;
static char* rew_file  = NULL;
static char* rewi_file = NULL;

/**
* This part simply prints the program usage info.
*/
static usage()
{
	printf("Usage: mrmc <logic> <options> <.tra file> <.lab file> <.rew file> <.rewi file>\n");
	printf("\t<logic>\t\t- could be one of {%s, %s, %s, %s}.\n",CSL_MODE_STR, PCTL_MODE_STR, PRCTL_MODE_STR, CSRL_MODE_STR);
	printf("\t<options>\t- could be one of {%s, %s}, optional.\n", F_IND_LUMP_MODE_STR, F_DEP_LUMP_MODE_STR);
	printf("\t<.tra file>\t- is the file with the matrix of transitions.\n");
	printf("\t<.lab file>\t- contains labeling.\n");
	printf("\t<.rew file>\t- contains state rewards (for PRCTL/CSRL).\n");
	printf("\t<.rewi file>\t- contains impulse rewards (for CSRL, optional).\n");
	printf("\nNote: In the '.tra' file transitions should be ordered by rows and columns!\n\n");
}

/**
* This part simply prints the programm Intro message
*/
static void print_intro()
{
	printf(" ------------------------------------------------------------------ \n");
	printf("|                    Markov Reward Model Checker                   |\n");
	printf("|                         MRMC version 1.3                         |\n");
	printf("|         Copyright (C) The University of Twente, 2004-2007.       |\n");
	printf("|               Copyright (C) RWTH-Aachen, 2006-2007.              |\n");
	printf("|                            Authors:                              |\n");
	printf("|   Joost-Pieter Katoen (since 2004), Ivan S Zapreev (since 2004), |\n");
	printf("|      Christina Jansen (since 2007), Tim Kemna (2005-2006),       |\n");
	printf("|                  Maneesh Khattri (2004-2005)                     |\n");
	printf("|          MRMC is distributed under the GPL conditions            |\n");
	printf("|          (GPL stands for GNU General Public License)             |\n");
	printf("|          The product comes with ABSOLUTELY NO WARRANTY.          |\n");
	printf("| This is a free software, and you are welcome to redistribute it. |\n");
	printf(" ------------------------------------------------------------------ \n");
}

/**
* This method simply tests matrix vector and vector matrix multiplication
* @param space this is an implicit parameter, which is a matrix to test
*/
static void testVMV()
{
	int i;
	const int N = space->rows;
	double *pV,*pR;
	struct timeval tt1, tt2;
	//Allocate arrays
	pV = (double *)calloc(N, sizeof(double));
	pR = (double *)calloc(N, sizeof(double));
	//Initialize the vector
	for(i=0; i < N; i++)
	{
		pV[i] = 2.0;
	}

	//Do computations M*v
	gettimeofday(&tt1, NULL);
	multiply_mtx_MV( space, pV, pR );
	gettimeofday(&tt2, NULL);
	printf("Time for M*v: %ld micro sec(s)\n",
		(tt2.tv_sec-tt1.tv_sec)*1000000+(tt2.tv_usec-tt1.tv_usec));

	//Do computations v*M
	gettimeofday(&tt1, NULL);
	multiply_mtx_TMV( space, pV, pR );
	gettimeofday(&tt2, NULL);
	printf("Time for v*M: %ld micro sec(s)\n",
		(tt2.tv_sec-tt1.tv_sec)*1000000+(tt2.tv_usec-tt1.tv_usec));
	free(pV);
	free(pR);
	free_sparse_ncolse(space);
}

/**
* This method determines the running mode depending on the command line parameters
* It checks whether we will work with CTMC or DTMC
* @return FALSE if it failed to recognize the input parameter as am MRMC
*	   mode parameter otherwise TRUE
*/
static BOOL setRunningMode(char * mode)
{
	BOOL result = FALSE;
	
	if( strcmp(mode,CSL_MODE_STR) == 0 ){
		result = TRUE;
		if( !isLogicRunModeSet() )
		{
			addRunMode(CSL_MODE);
			print_run_mode( TRUE, FALSE );
		}else{
			printf("WARNING: The Logic mode has already been set, the parameter '%s' is skipped.\n", mode);
		}
	}else if( strcmp(mode,PCTL_MODE_STR) == 0  && !isLogicRunModeSet() ){
		result = TRUE;
		if( !isLogicRunModeSet() )
		{
			addRunMode(PCTL_MODE);
			print_run_mode( TRUE, FALSE );
		}else{
			printf("WARNING: The Logic mode has already been set, the parameter '%s' is skipped.\n", mode);
		}
	}else if( strcmp(mode,PRCTL_MODE_STR) == 0  && !isLogicRunModeSet() ){
		result = TRUE;
		if( !isLogicRunModeSet() )
		{
			addRunMode(PRCTL_MODE);
			print_run_mode( TRUE, FALSE );
		}else{
			printf("WARNING: The Logic mode has already been set, the parameter '%s' is skipped.\n", mode);
		}
	}else if( strcmp(mode,CSRL_MODE_STR) == 0  && !isLogicRunModeSet() ){
		result = TRUE;
		if( !isLogicRunModeSet() )
		{
			addRunMode(CSRL_MODE);
			print_run_mode( TRUE, FALSE );
		}else{
			printf("WARNING: The Logic mode has already been set, the parameter '%s' is skipped.\n", mode);
		}
	}else if( strcmp(mode,TEST_VMV_MODE_STR) == 0 && !isLogicRunModeSet() ){
		result = TRUE;
		if( !isLogicRunModeSet() )
		{
			addRunMode(TEST_VMV_MODE);
			print_run_mode( TRUE, FALSE );
		}else{
			printf("WARNING: The Logic mode has already been set, the parameter '%s' is skipped.\n", mode);
		}
	}else if( strcmp(mode,F_IND_LUMP_MODE_STR) == 0 ){
		result = TRUE;
		if( !isRunMode(F_DEP_LUMP_MODE) ){
			addRunMode(F_IND_LUMP_MODE);
			print_run_mode( FALSE, TRUE );
		}else{
			printf("WARNING: The lumping mode has already been set, the parameter '%s' is skipped.\n", mode);
		}
	}else if( strcmp(mode,F_DEP_LUMP_MODE_STR) == 0 ){
		result = TRUE;
		if( !isRunMode(F_IND_LUMP_MODE) ){
			addRunMode(F_DEP_LUMP_MODE);
			print_run_mode( FALSE, TRUE );
		}else{
			printf("WARNING: The lumping mode has already been set, the parameter '%s' is skipped.\n", mode);
		}
	}
	return result;
}

/**
* This function should find the extension and then return it and its lenght.
* @param p_pot_filename the input parameter which we suspect to be a file name
* @param p_extension the pointer to an extension string (a return parameter)
* @param p_ext_length the pointer to the extension string length (a return parameter)
* @return TRUE if we can subtract a file extension which we think is one of required
*/
static BOOL isValidExtension(char* p_pot_filename, char * p_extension, int * p_ext_length)
{
	BOOL result = FALSE;
	char *p;
	int length;
	
	length = strlen(p_pot_filename);
	p = strrchr(p_pot_filename,'.');     // The last occurance of '.' in the file name
	*p_ext_length = length - (p - p_pot_filename); //including '.', excluding '\0'
	
	if( length >= MIN_FILE_NAME_LENGTH &&
		( *p_ext_length == MAX_FILE_EXT_LENGTH ||
		  *p_ext_length == ( MAX_FILE_EXT_LENGTH - 1 ) ) ){
		//Get the extention
		if( *p_ext_length == ( MAX_FILE_EXT_LENGTH - 1 ) ){
			strncpy( p_extension, p , MAX_FILE_EXT_LENGTH );
		}else{
			strncpy( p_extension, p , MAX_FILE_EXT_LENGTH + 1 );
		}
		result = TRUE;
	}
	
	return result;
}

/**
* Load the .rewi impulse rewards file
*/
void loadImpulseRewards( char *file_name ){
	if( is_rewi_present ){
		if ( isRunMode(CSRL_MODE) )
		{
			
			printf("Loading the '%s' file, please wait.\n", file_name);
			sparse * rewi = read_impulse_rewards(file_name, space->rows);
			if ( rewi == NULL )
			{
				printf("ERROR: The '%s' file '%s' was not found.\n", REWI_FILE_EXT, file_name);
				exit(1);
			}
			setImpulseRewards(rewi);
		}else{
			printf("WARNING: Impulse rewards are not supported in the current mode, skipping the '%s' file loading.\n", file_name);
		}
	}
}

/**
* Load the .rew state rewards file
*/
void loadStateRewards( char * file_name ){
	if( is_rew_present ){
		if ( isRunMode(PRCTL_MODE) || isRunMode(CSRL_MODE) )
		{
			printf("Loading the '%s' file, please wait.\n", file_name);
			double * rew = read_rew_file(space->rows, file_name);
			if ( rew == NULL )
			{
				printf("ERROR: The '%s' file '%s' was not found.\n", REW_FILE_EXT, file_name);
				exit(1);
			}
			setStateRewards(rew);
		}else{
			printf("WARNING: State rewards are not supported in the current mode, skipping the '%s' file loading.\n", file_name);
		}
	}
}

/**
* Load the .tra transitions file
*/
void loadTransitions(char * file_name){
	if( is_tra_present ){
		printf("Loading the '%s' file, please wait.\n", file_name);
		space = read_tra_file(file_name);
		if( space == NULL )
		{
			printf("ERROR: The '%s' file '%s' was not found.\n", TRA_FILE_EXT, file_name);
			exit(1);
		}
		set_state_space(space);
	}
}

/**
* Load the .lab labelling file
* WARNING: Presumes that transitions have been loaded and
* sparse variable is properly initialized
*/
loadLabels( char * file_name){
	if( is_lab_present ){
		printf("Loading the '%s' file, please wait.\n", file_name);
		labels = read_lab_file(space->rows, file_name);
		if(labels == NULL)
		{
			printf("ERROR: The '%s' file '%s' was not found.\n", LAB_FILE_EXT, file_name);
			exit(1);
		}
		set_labeller(labels);
	}
}

/**
* This method is used to check that all required files
* are present in the command line parameters and also that other
* command line options are used in valid combinations.
*/
static void checkConsystency(){
	//Check for the logic preset
	if( !isLogicRunModeSet() ){
		printf("ERROR: The <logic> parameter is undefined.\n");
		usage();
		exit(1);
	}
	
	//Check for the option combinations.
	//Formula independent lumping for CSRL is not supported, the option will be ignored.
	if( isRunMode(CSRL_MODE) && isRunMode(F_IND_LUMP_MODE) && is_rewi_present){
		printf("WARNING: Formula independent lumping for CSRL with impulse rewards is not supported, the '%s' option will be ignored.\n",F_IND_LUMP_MODE_STR);
		clearRunMode(F_IND_LUMP_MODE);
		printf("The formula independent lumping is OFF.\n");
	}
	//Formula dependent lumping in CSRL is not supported for the case with impulse rewards
	if( isRunMode(CSRL_MODE) && isRunMode(F_DEP_LUMP_MODE) && is_rewi_present ){
		printf("WARNING: Formula dependent lumping for CSRL with impulse rewards is not supported, the '%s' option will be ignored.\n",F_DEP_LUMP_MODE_STR);
		clearRunMode(F_DEP_LUMP_MODE);
		printf("The formula dependent lumping is OFF.\n");
	}
	//Formula independent lumping in PRCTL is not supported for the case with impulse rewards
	if( isRunMode(PRCTL_MODE) && isRunMode(F_IND_LUMP_MODE) && is_rewi_present ){
		printf("WARNING: Formula independent lumping for PRCTL with impulse rewards is not supported, the '%s' option will be ignored.\n",F_IND_LUMP_MODE_STR);
		clearRunMode(F_IND_LUMP_MODE);
		printf("The formula independent lumping is OFF.\n");
	}
	//Formula dependent lumping in PRCTL is not supported for the case with impulse rewards
	if( isRunMode(PRCTL_MODE) && isRunMode(F_DEP_LUMP_MODE) && is_rewi_present ){
		printf("WARNING: Formula dependent lumping for PRCTL with impulse rewards is not supported, the '%s' option will be ignored.\n",F_DEP_LUMP_MODE_STR);
		clearRunMode(F_DEP_LUMP_MODE);
		printf("The formula dependent lumping is OFF.\n");
	}

	//Check for the presence of all required files.
	char *missing_file = NULL;
	if( !is_tra_present ){
		missing_file = TRA_FILE_EXT;
	}else if( !is_lab_present ){
		missing_file = LAB_FILE_EXT;
	}else if( ( isRunMode(PRCTL_MODE) || isRunMode(CSRL_MODE) ) && !is_rew_present ){
		missing_file = REW_FILE_EXT;
	}
	if( missing_file != NULL ){
		printf("ERROR: The '%s' file is required to be present.\n", missing_file);
		usage();
		exit(1);
	}
}

/**
* This method is used for parsing the input paramters of the MRMC tool
* and also for loading the data files
* NOTE: This method terminates the programm in case of incorrect input parameters
*/
static void parseInParamsAndInitialize(int argc, char *argv[])
{
	int i = 0, ext_length;
	char expension[MAX_FILE_EXT_LENGTH+1]; //+1 because we need also to store the \0 symbol

	//NOTE: Note it is important to load files in the following order:
	//	  .tra, .lab, .rew, .rewi
	//So we first parse the input parameters, sort them out and only
	//then read the files.
	for(i = 1; i < argc; i++)
	{
		if( !setRunningMode( argv[i] ) ){
			if( isValidExtension( argv[i], expension, &ext_length ) ){
				if( strcmp(expension, TRA_FILE_EXT) == 0 ){
					if( !is_tra_present ){
							is_tra_present = TRUE;
							tra_file = argv[i];
					}else{
						printf("WARNING: The '%s' file has been noticed before, skipping the '%s' file.\n", tra_file, argv[i]);
					}
				}else if ( strcmp(expension, LAB_FILE_EXT) == 0 ){
					if( !is_lab_present ){
							is_lab_present = TRUE;
							lab_file = argv[i];
					}else{
						printf("WARNING: The '%s' file has been noticed before, skipping the '%s' file.\n", lab_file, argv[i]);
					}
				}else if ( strcmp(expension, REW_FILE_EXT) == 0 ){
					if( !is_rew_present ){
							is_rew_present = TRUE;
							rew_file = argv[i];
					}else{
						printf("WARNING: The '%s' file has been noticed before, skipping the '%s' file.\n", rew_file, argv[i]);
					}
				}else if ( strcmp(expension, REWI_FILE_EXT) == 0 ){
					if( !is_rewi_present ){
							is_rewi_present = TRUE;
							rewi_file = argv[i];
					}else{
						printf("WARNING: The '%s' file has been noticed before, skipping the '%s' file.\n", rewi_file, argv[i]);
					}
				}else{
					printf("WARNING: An unknown file type '%s' for input file '%s', skipping.\n", expension, argv[i]);
				}
			} else {
				printf("WARNING: An unknown logic, option or file '%s', skipping.\n", argv[i]);
			}
		}
	}

	//Check that for the parameters consystency,
	//may cause the program termination.
	checkConsystency();

	//Load the files if present
	loadTransitions( tra_file );
	loadLabels( lab_file );
	loadStateRewards( rew_file );
	loadImpulseRewards( rewi_file);

	//If we are in the test mode then just test and exit.
	if( isRunMode(TEST_VMV_MODE) )
	{
		//The tra file has to be preloaded already
		testVMV();
		exit(0);
	}
}

/**
* Does formula independent lumping if the isRunMode(F_IND_LUMP_MODE_STR) == TRUE
*/
void doFormulaIndependentLumping(){
	if( isRunMode( F_IND_LUMP_MODE ) ){
		printf("Formula Independent Lumping...\n");
		//Starting the timer, if it has not been started yet
		startTimer();
		
		//Create initial partitioning
		partition * P = init_partition(labels);
		
		//Lump the state space
		sparse *Q1 = lump(P, space);
		
		//Change labelling
		change_labelling(labels, P);
		
		//Change state rewards
		double * p_old_rew, * p_new_rew;
		p_old_rew = getStateRewards();
		p_new_rew = change_state_rewards(p_old_rew, P);
		freeStateRewards();
		setStateRewards(p_new_rew);
		
		// free the original state space
		free_sparse_ncolse(space);
		free_row_sums();
		//Reset the state space
		space = Q1;
		//Store into the runtime.c
		set_state_space(space);
		setPartition(P);
		
		//Stop timer and print the elapsed time
		const long elapsed_time = stopTimer();
		printf("The Total Elapsed Lumping Time is %ld milli sec(s).\n", elapsed_time);
	}
}

main(int argc, char *argv[])
{
	unsigned int sp1=0,sp2=0;

	//Pring the introduction message
	print_intro();

	#ifndef __APPLE__
		sp1 = mallinfo().uordblks;
	#endif
	
	//Parse and validate the input parameters,
	//load required files
	parseInParamsAndInitialize(argc, argv);
	
	#ifndef __APPLE__
		sp2 = mallinfo().uordblks;

		printf("The Occupied Space is %d Bytes.\n", (sp2-sp1));
	#else
		printf("The Occupied Space is ??? Bytes.\n");
	#endif
	
	//Do formula independent lumping, if required
	doFormulaIndependentLumping();

	printf("Type 'help' to get help.\n>>");
	
	while( yyparse() )
	{
		printf(">>");
	}
	free_sparse_ncolse(space);
	freeImpulseRewards();
	free_labelling(labels);
	freeStateRewards();
	free_row_sums();
	freePartition();
}
