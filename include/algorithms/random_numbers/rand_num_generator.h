/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: rand_num_generator.h,v $
*	$Revision: 1.8 $
*	$Date: 2007/10/07 12:42:23 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2007.
*	Authors: Ivan Zapreev, Christina Jansen
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
*	Source description: This file contains functions for generating random numbers.
*	
*	WARNING: For all RNG methods, except for ones using GSL the discrete and exponential
*	random variables are based on the same seed. We do not know whether it has any
*	influence on the distribution of the generated random variables.
*/

#include "macro.h"

#ifndef RANDOM_NON_UNIF_H
#define RANDOM_NON_UNIF_H

/**
* The available random number generators for the simulation.
* (1) Combined linear congruential generator taken from "Applied
*	Cryptography" by B. Schneier
* (2) Linear congruential generator used in Prism
* (3) Lehmer generator written by Park & Geyer
* (4) Mersenne twister used in Ymer
* (5) Version of the RANLUX algorithm of Lüscher from GNU Scientific Library
* (6) A Lagged Fibonacci generator (LFG) from GNU Scientific Library
* (7) A Tausworthe generator from GNU Scientific Library
*/
#define RNG_APP_CRYPT_METHOD	1
#define RNG_METHOD_APP_CRYPT_STR "Appl. Crypt."

#define RNG_PRISM_METHOD	2
#define RNG_METHOD_PRISM_STR "Prism"

#define RNG_CIARDO_METHOD	3
#define RNG_METHOD_CIARDO_STR "Ciardo"

#define RNG_YMER_METHOD		4
#define RNG_METHOD_YMER_STR "Ymer"

#define RNG_GSL_RANLUX_METHOD	5
#define RNG_METHOD_GSL_RANLUX_STR "CSL Ranlux"

#define RNG_GSL_LFG_METHOD	6
#define RNG_METHOD_GSL_LFG_STR "GSL Lfg"

#define RNG_GSL_TAUS_METHOD	7
#define RNG_METHOD_GSL_TAUS_STR "GSL Taus"

#define RNG_METHOD_UNKNOWN_STR "????"

/************************************************************************/
/*************************Discrete Distribution**************************/
/************************************************************************/

/**
* Sets the method for computing non-uniformly distributed random numbers for
* the discrete distribution.
* @param _method one from {RNG_APP_CRYPT_METHOD, RNG_PRISM_METHOD, 
* RNG_CIARDO_METHOD, RNG_YMER_METHOD, RNG_GSL_RANLUX_METHOD, RNG_GSL_LFG_METHOD,
* RNG_GSL_TAUS_METHOD}
*/
extern void setRNGMethodDiscrete(int _method);

/**
* Returns the selected method for computing non-uniformly distributed random
* numbers for the discrete distribution.
* NOTE: method should be one from {RNG_APP_CRYPT_METHOD, RNG_PRISM_METHOD, 
* RNG_CIARDO_METHOD, RNG_YMER_METHOD, RNG_GSL_RANLUX_METHOD, RNG_GSL_LFG_METHOD,
* RNG_GSL_TAUS_METHOD}
*/
extern inline int getRNGMethodDiscrete();

/**
* This functions generates a new seed for the selected method for
* non-uniformly distributed random numbers for discrete distribution.
* NOTE: The stream for generating random numbers is seeded by the system clock!	
* WARNING: Unless GSL is used this regenerates the seed for the exp. distr. rand.
* variables as well!
*/
extern inline void generateNewSeedDiscrete();

/**
* This function chooses a random value from the given array according
* to the related probability distribution.
* @param values the range of values one would like to choose from
* @param prob the related probability distribution
* @param size the size of the values and prob arrays
*/
extern int generateRandNumberDiscrete(int *values, double *_distribution, int _size);

/**
* This function frees all memory allocated for the random number generator
* used for the discrete distribution.
*/
extern inline void freeRNGDiscrete();

/************************************************************************/
/***********************Exponential Distribution*************************/
/************************************************************************/

/**
* Sets the method for computing non-uniformly distributed random numbers for
* the exponential distribution.
* @param _method one from {RNG_APP_CRYPT_METHOD, RNG_PRISM_METHOD, 
* RNG_CIARDO_METHOD, RNG_YMER_METHOD, RNG_GSL_RANLUX_METHOD, RNG_GSL_LFG_METHOD,
* RNG_GSL_TAUS_METHOD}
*/
extern void setRNGMethodExp(int _method);

/**
* Returns the selected method for computing non-uniformly distributed random
* numbers for the exponential distribution.
* NOTE: method should be one from {RNG_APP_CRYPT_METHOD, RNG_PRISM_METHOD, 
* RNG_CIARDO_METHOD, RNG_YMER_METHOD, RNG_GSL_RANLUX_METHOD, RNG_GSL_LFG_METHOD,
* RNG_GSL_TAUS_METHOD}
*/
extern inline int getRNGMethodExp();

/**
* This functions generates a new seed for the selected method for
* non-uniformly distributed random numbers for exponential distribution.
* NOTE: The stream for generating random numbers is seeded by the system clock!	
* WARNING: Unless GSL is used this regenerates the seed for the discrete
* rand. variables as well!
*/
extern inline void generateNewSeedExp();

/**
* Get a random exponential distributed number 
* @param lambda: inverse scale of exponential distribution
* @return the random number (exponential distributed)
*/
extern inline double generateRandNumberExp(double lambda);

/**
* This function frees all memory allocated for the random number generator
* used for the exponential distribution.
*/
extern inline void freeRNGExp();

/****************************************************************************/
/*******************PRINT THE SIMULATION RUNTIME PARAMETERS******************/
/****************************************************************************/

/**
* Print the random-number generator runtime parameters.
*/
/**
* Print the random-number generator runtime parameters for discrete r.v.
*/
extern inline void printRuntimeRNGInfoDiscrete();

/**
* Print the random-number generator runtime parameters for exponential r.v.
*/
extern inline void printRuntimeRNGInfoExp();

#endif
