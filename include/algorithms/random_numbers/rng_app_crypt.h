/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: rng_app_crypt.h,v $
*	$Revision: 1.4 $
*	$Date: 2007/10/01 16:21:52 $
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

#include "macro.h"

#ifndef APP_CRYPT_H
#define APP_CRYPT_H

/**
* Choose uniformly random number between 0 and 1, approach taken from 
* "Applied Cryptography" by Bruce Schneier 
* @param pRandGenerator the rand number generator structure, needed for GSL.
* @return a random value between 0 and 1
*/
extern double generateRandUnifAC(void * pRandGenerator);

/**
* This function seeds the random number generator.
* @param pRandGenerator the rand number generator structure, needed for GSL.
* @param _s1 the value the stream of random numbers is started with
*/
extern void generateSeedAC(void * pRandGenerator, unsigned long _s1);

#endif


