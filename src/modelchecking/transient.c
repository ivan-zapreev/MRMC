/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: transient.c,v $
*	$Revision: 1.11 $
*	$Date: 2007/10/02 11:50:54 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Maneesh Khattri, Ivan Zapreev, Tim Kemna
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
*	Source description: Perform Transient Analysis for PCTL/CSL/PRCTL/CSRL - X, U.
*/

#include "transient.h"

/**
* Solve until formula.
* @param type one of: UNBOUNDED_FORM, INTERVAL_FORM
* @param phi satisfaction relation for phi formula.
* @param psi satisfaction relation for psi formula.
* @param subi sub I (time).
* @param supi sup I (time).
* @param isIgnoreFLumping if we want the formula dependent lumping to be ignored
* @return result of the until formula for all states.
*/
double * until(const int type, bitset *phi, bitset *psi, double subi, double supi, BOOL isIgnoreFLumping)
{
	const BOOL isLumping = isRunMode(F_DEP_LUMP_MODE) && ( ! isIgnoreFLumping );
	
	if( isRunMode(CSL_MODE) || isRunMode(CSRL_MODE))
	{
		/*The CTMC case*/
		if( type == TIME_UNBOUNDED_FORM ){
			return ( isLumping ? unbounded_until_lumping(phi, psi) : unbounded_until(phi, psi) );
		}else{
			if( type == TIME_INTERVAL_FORM ){
				if( subi == 0.0 ){
					return ( isLumping ? bounded_until_lumping(phi, psi, supi) : bounded_until(phi, psi, supi) );
				}else{
					if( subi > 0.0 && supi >= subi ){
						return ( isLumping ? interval_until_lumping(phi, psi, subi, supi) : interval_until(phi, psi, subi, supi) );
					}else{
						printf("ERROR: The formula time bounds [%lf,%lf] are inappropriate.\n", subi, supi);
						return (double *)calloc(phi->n, sizeof(double));
					}
				}
			}else{
				printf("ERROR: An unknown Until formula type %d.\n", type);
				exit(1);
			}
		}
	}else{
		if( isRunMode(PCTL_MODE) || isRunMode(PRCTL_MODE)){
			/*The DTMC case*/
			if( type == TIME_UNBOUNDED_FORM ){
				return ( isLumping ? dtmc_unbounded_until_lumping(phi, psi) : dtmc_unbounded_until(phi, psi) );
			}else{
				if( type == TIME_INTERVAL_FORM ){
					if( subi == 0.0 ){
						return ( isLumping ? dtmc_bounded_until_lumping(phi, psi, supi) : dtmc_bounded_until(phi, psi, supi) );
					}else{
						printf("ERROR: This formula with general time bounds is not supported for PCTL.\n");
						return (double *)calloc(phi->n, sizeof(double));
					}
				}else{
					printf("ERROR: An unknown Until formula type %d.\n", type);
					exit(1);
				}
			}
		}else{
			printf("ERROR: The logic does not support the Until formula.\n");
			exit(1);
		}
	}
	//NOTE: This should not be happening
	return NULL;
}

/**
* Solve next formula.
* @param type one of: TIME_UNBOUNDED_FORM, TIME_INTERVAL_FORM
* @param phi satisfaction relation for phi formula.
* @param subi sub I (time).
* @param supi sup I (time).
* @return result of the next formula for all states.
*/
double * next(const int type, bitset *phi, double subi, double supi){
	if( isRunMode(CSL_MODE) || isRunMode(CSRL_MODE) ){
		/*The CTMC case*/
		if( type == TIME_UNBOUNDED_FORM ){
			return unbounded_next(phi);
		}else{
			if( type == TIME_INTERVAL_FORM ){
				if( subi == 0.0){
					return bounded_next(phi, supi);
				}else{
					if( subi > 0.0 && supi >= subi ){
						return interval_next(phi, subi, supi);
					}else{
						printf("ERROR: The formula time bounds [%lf,%lf] are inappropriate.\n", subi, supi);
						return (double *)calloc(phi->n, sizeof(double));
					}
				}
			}else{
				printf("ERROR: An unknown Next formula type %d.\n", type);
				exit(1);
			}
		}
	}else{
		if( isRunMode(PCTL_MODE) || isRunMode(PRCTL_MODE)  ){
			/*The DTMC case*/
			if( type == TIME_UNBOUNDED_FORM ){
				return dtmc_unbounded_next(phi);
			}else{
				printf("ERROR: Formula is not supported in PCTL mode.\n");
				return (double *)calloc(phi->n, sizeof(double));
			}
		}else{
			printf("ERROR: The logic does not support the Next formula.\n");
			exit(1);
		}
	}
	//NOTE: This should not be happening
	return NULL;
}

/**
* Solve until formula with rewards.
* @param phi satisfaction relation for phi formula.
* @param psi satisfaction relation for psi formula.
* @param subi sub I (time).
* @param supi sup I (time).
* @param subj sub J (reward).
* @param supj sup J (reward).
* @param isIgnoreFLumping if we want the formula dependent lumping to be ignored
* @return result of the until formula for all states.
*/
double * until_rewards(bitset *phi, bitset *psi, double subi, double supi, double subj, double supj, BOOL isIgnoreFLumping)
{
	const BOOL isLumping = isRunMode(F_DEP_LUMP_MODE) && ( ! isIgnoreFLumping );
	
	if( isRunMode(CSRL_MODE) ){
		if( subi == 0.0 && subj == 0.0 && supi != 0.0 && supj != 0.0 ){
			return ( isLumping ? ctmrm_bounded_until_lumping(phi, psi, supi, supj) : ctmrm_bounded_until(phi, psi, supi, supj) );
		}else{
			printf("ERROR: Given parameters combination is not supported by CSRL.\n");
			return (double *)calloc(phi->n, sizeof(double));
		}
	}else{
		if( isRunMode(PRCTL_MODE) ){
			if( supi != 0.0 && supj != 0.0 ){
				return ( isLumping ? dtmrm_bounded_until_lumping(phi, psi, subi, supi, subj, supj) : dtmrm_bounded_until(phi, psi, subi, supi, subj, supj) );
			}else{
				printf("ERROR: Given parameters combination is not supported by PRCTL.\n");
				return (double *)calloc(phi->n, sizeof(double));
			}
		}else{
			printf("ERROR: The logic does not support the Until formula.\n");
			exit(1);
		}
	}
	//NOTE: This should not be happening
	return NULL;
}

/**
* Solve next_rewards formula.
* @param phi satisfaction relation for phi formula.
* @param subi sub I (time).
* @param supi sup I (time).
* @param subj sub J (reward).
* @param supj sup J (reward).
* @return result of the next formula for all states.
* NOTE: ********without impulse rewards at present***********
* 		  ********to be tested***********
*/
double * next_rewards(bitset *phi, double subi, double supi, double subj, double supj)
{
	if( isRunMode(CSRL_MODE) ){
		/*The CMRM case*/
		if( supi != 0.0 ){
			return ctmrm_interval_next(phi, subi, supi, subj, supj);
		}else{
			printf("ERROR: Given parameters combination is not supported by CSRL.\n");
			return (double *)calloc(phi->n, sizeof(double));
		}
	}else{
		if( isRunMode(PRCTL_MODE) ){
			/*Only unbounded next is allowed (see parser for
			restriction of other cases) - The DMRM case*/
			if(subi==0 && supi==0 && subj==0 && supj==0){
				return dtmc_unbounded_next(phi);
			}else{
				printf("ERROR: Formula is not supported in PRCTL mode.\n");
				return (double *)calloc(phi->n, sizeof(double));
			}
		}else{
			printf("ERROR: The logic does not support the Next formula.\n");
			exit(1);
		}
	}
}
