/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: rng_app_crypt.c,v $
*	$Revision: 1.4 $
*	$Date: 2007/10/15 09:04:57 $
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
*		A C library for generating random numbers.
*		The method used for generating random numbers is a combined linear 
*		congruential one and returns uniformly distributed random numbers
*		between 0 and 1.
*		
*		For more information see: "Applied Cryptography", Bruce Schneier,
*		John Wiley & Sons, 1996
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "rng_app_crypt.h"

/* Macro for calculating parameters for choosing a uniformly random number,
 approach taken from Applied Cryptography", Bruce Schneier */ 
#define MODMULT(a, b, c, m, s) q = s/a; s = b*(s-a*q)-c*q; if (s < 0) s += m;

static long s1, s2;

/**
* Choose uniformly random number between 0 and 1, approach taken from 
* "Applied Cryptography" by Bruce Schneier 
* @param pRandGenerator the rand number generator structure, needed for GSL.
* @return a random value between 0 and 1
*/
double generateRandUnifAC(void * pRandGenerator){
	long q, z;
	
	MODMULT(53668, 40014, 12211, 2147483563L, s1)
	MODMULT(52774, 40692, 3791, 2147483399L, s2)
	z = s1 - s2;
	if (z < 1){
		z += 2147483562;
	}
	return z * 4.656613e-10;
}

/**
* This function seeds the random number generator.
* @param pRandGenerator the rand number generator structure, needed for GSL.
* @param _s1 the value the stream of random numbers is started with
*/
void generateSeedAC(void * pRandGenerator, unsigned long _s1){
	srand( (unsigned)time( NULL) );
	s1 = _s1;
	s2 = rand();
}
