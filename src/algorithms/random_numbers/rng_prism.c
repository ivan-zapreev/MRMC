/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: rng_prism.c,v $
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
*		A C library for generating random numbers. It mostly relys on
*		the common C standard functions rand() and srand().
*		The method used for generating random numbers is a linear 
*		congruential one and returns uniformly distributed random numbers
*		between 0 and 1.
*		
*		For more information see: "man rand"
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "rng_prism.h"

/**
* Create uniformly distributed random number between 0 and 1.
* @param pRandGenerator the rand number generator structure, needed for GSL.
*/
inline double generateRandUnifPR(void * pRandGenerator){
	return (double) rand()/(RAND_MAX+1.0);
}

/**
* This function seeds the linear congruential random number
* generator.
* @param pRandGenerator the rand number generator structure, needed for GSL.
* @param s the value the stream of random numbers is started with
*/
inline void generateSeedPR(void * pRandGenerator, unsigned long s){
	srand(s);
}
