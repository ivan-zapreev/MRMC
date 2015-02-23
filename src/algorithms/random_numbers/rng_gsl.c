/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: rng_gsl.c,v $
*	$Revision: 1.4 $
*	$Date: 2007/10/03 09:53:58 $
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
*	Source description:
*		A wrapper file for using different random number generators from the
*		GNU Scientific Library.
*			
*		For more information see:
*			http://www.gnu.org/software/gsl/
*/

#include "gsl/gsl_rng.h"
#include "gsl/gsl_randist.h"
#include "rand_num_generator.h"

#include "rng_gsl.h"

/**
* Sets the method for computing non-uniformly distributed random numbers - 
* all implementations taken from the GNU Scientific Library.
* @param _method one from {RNG_GSL_RANLUX_METHOD, RNG_GSL_LFG_METHOD, RNG_GSL_TAUS_METHOD}
* @return the pointer to the random number generator instance
*/
void * setGSLMethod(int _method){
	gsl_rng * pRandGenerator = NULL;
	
	switch( _method ){
		case RNG_GSL_RANLUX_METHOD:
			pRandGenerator = gsl_rng_alloc( gsl_rng_ranlxs2 );
			break;
		case RNG_GSL_LFG_METHOD:
			pRandGenerator = gsl_rng_alloc( gsl_rng_gfsr4 );
			break;
		case RNG_GSL_TAUS_METHOD:
			pRandGenerator = gsl_rng_alloc( gsl_rng_taus2 );
			break;
		default:
			printf("ERROR: Invalid method for computing non-uniformly distributed random numbers!\n");
			exit(1);
	}
	return (void *) pRandGenerator;
}

/**
* This function seeds the choosen GSL random number
* generator.
* @param pRandGenerator the rand number generator structure, needed for GSL.
* @param s the value the stream of random numbers is started with
*/
inline void generateSeedGSL(void * pRandGenerator, unsigned long s){
	gsl_rng_set( (gsl_rng *) pRandGenerator, s );
}

/**
* Create uniformly distributed random number between 0 and 1 
* @param pRandGenerator the rand number generator structure, needed for GSL.
* @return random number between 0 and 1
*/
inline double generateRandUnifGSL(void * pRandGenerator){
	return gsl_rng_uniform_pos( (gsl_rng *) pRandGenerator );
}

/**
* Creates an exponentially distributed random number using the
* choosen GSL random number generator.
* @param pRandGenerator the rand number generator structure, needed for GSL.
* @param lambda: inverse scale of exponential distribution
* @return the random number (exponential distributed)
*/
inline double generateExpRandNumberGSL(void * pRandGenerator, double lambda){
	return gsl_ran_exponential( (gsl_rng *) pRandGenerator, 1.0/lambda );
}

/**
* This function frees the memory allocated for the choosen random number generator.
* @param pRandGenerator the rand number generator structure, needed for GSL.
*/
inline void freeRNGGSL(void * pRandGenerator){
	gsl_rng_free( (gsl_rng *) pRandGenerator );
}
