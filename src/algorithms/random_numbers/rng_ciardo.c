/**
*	WARNING: Do Not Remove This Section
*	
*	$RCSfile: rng_ciardo.c,v $
*	$Revision: 1.4 $
*	$Date: 2007/10/03 09:53:57 $
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
*		This is an ANSI C library for random number generation.  The use of this 
*		library is recommended as a replacement for the ANSI C rand() and srand()
*		functions, particularly in simulation applications where the statistical
*		'goodness' of the random number generator is important.
*		
*		The generator used in this library is a so-called 'Lehmer random number
*		generator' which returns a pseudo-random number uniformly distributed
*		between 0.0 and 1.0.  The period is (m - 1) where m = 2,147,483,647 and
*		the smallest and largest possible values are (1 / m) and 1 - (1 / m)
*		respectively.  For more details see:
*	 
*		"Random Number Generators: Good Ones Are Hard To Find"
*				Steve Park and Keith Miller
*		Communications of the ACM, October 1988
*		
*		Note that as of 7-11-90 the multiplier used in this library has changed
*		from the previous "minimal standard" 16807 to a new value of 48271.  To
*		use this library in its old (16807) form change the constants MULTIPLIER
*		and CHECK as indicated in the comments.
*		
*		Name              : rng_ciardo.c  (Random Number Generation - Single Stream)
*		Authors           : Steve Park & Dave Geyer
*		Language          : ANSI C
*		Latest Revision   : 09-11-98
*/

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include "rng_ciardo.h"

#define MODULUS    2147483647L /* DON'T CHANGE THIS VALUE                   */
#define MULTIPLIER 48271L      /* use 16807 for the "minimal standard"      */
#define CHECK      399268537L  /* use 1043616065 for the "minimal standard" */
#define DEFAULT    123456789L  /* initial seed, use 0 < DEFAULT < MODULUS   */

static long seed = DEFAULT;    /* seed is the state of the generator        */

/**
* generateRandUnifC is a Lehmer generator that returns a pseudo-random real number
* uniformly distributed between 0.0 and 1.0.  The period is (m - 1)
* where m = 2,147,483,647 amd the smallest and largest possible values
* are (1 / m) and 1 - (1 / m) respectively.
* @param pRandGenerator the rand number generator structure, needed for GSL.
* @return a random value between 0 and 1
*/
double generateRandUnifC(void * pRandGenerator){
	const long Q = MODULUS / MULTIPLIER;
	const long R = MODULUS % MULTIPLIER;
	long t;
	
	t = MULTIPLIER * (seed % Q) - R * (seed / Q);
	if (t > 0){
		seed = t;
	}else{
		seed = t + MODULUS;
	}
	return ((double) seed / MODULUS);
}


/**
* Use this (optional) procedure to initialize or reset the state of
* the random number generator according to the following conventions:
* if x > 0 then x is the initial seed (unless too large)
* if x < 0 then the initial seed is obtained from the system clock
* if x = 0 then the initial seed is to be supplied interactively
* @param pRandGenerator the rand number generator structure, needed for GSL.
* @param x the value the stream of random numbers should be started with
*/
void generateSeedC(void * pRandGenerator, unsigned long x){
	char ok = 0;
	
	if (x > 0L){
		/* correct if x is too large  */
		x = x % MODULUS;
	}
	if (x < 0L){
		x = ((unsigned long) time((time_t *) NULL)) % MODULUS;
	}
	if (x == 0L){
		while (!ok){
			printf("\nEnter a positive integer seed (9 digits or less) >> ");
			scanf("%ld", &x);
			ok = (0L < x) && (x < MODULUS);
			if (!ok){
				printf("\nInput out of range ... try again\n");
			}
		}
	}
	seed = x;
}
