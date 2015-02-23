/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: rng_ymer.h,v $
*	$Revision: 1.6 $
*	$Date: 2007/10/01 16:21:53 $
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
*		A C-program for MT19937, with initialization improved 2002/2/10.
*		Coded by Takuji Nishimura and Makoto Matsumoto.
*		This is a faster version by taking Shawn Cokus's optimization,
*		Matthe Bellew's simplification, Isaku Wada's real version.
*		
*		Before using, initialize the state by using init_genrand(seed).
*		
*		Modified for Ymer 11/11/2003.
*		
*		Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
*		All rights reserved.                          
*		
*		Redistribution and use in source and binary forms, with or without
*		modification, are permitted provided that the following conditions
*		are met:
*		
*		1. Redistributions of source code must retain the above copyright
*		        notice, this list of conditions and the following disclaimer.
*		
*		2. Redistributions in binary form must reproduce the above copyright
*		        notice, this list of conditions and the following disclaimer in the
*		        documentation and/or other materials provided with the distribution.
*		
*		3. The names of its contributors may not be used to endorse or promote 
*		        products derived from this software without specific prior written 
*		        permission.
*		
*		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*		"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*		LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*		A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
*		CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*		PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
*		PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
*		LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
*		NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*		SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*		
*		Any feedback is very welcome.
*			http://www.math.keio.ac.jp/matumoto/emt.html
*		email: matumoto@math.keio.ac.jp
*/

#include "macro.h"

#ifndef RNG_YMER_H
#define RNG_YMER_H

typedef struct {
  unsigned long aaa;
  unsigned long maskB, maskC;
  unsigned long *state;
  int left, initf;
  unsigned long *next;
} mt_struct;

/**
* Generates a random number in the interval (0,1).
* @param pRandGenerator the rand number generator structure, needed for GSL.
*/
extern double generateRandUnifYM(void * pRandGenerator);

/**
* Seeds the MT19937 random number generator.
* @param pRandGenerator the rand number generator structure, needed for GSL.
* @param s the value the stream of random numbers should start with
*/
extern void generateSeedYM(void * pRandGenerator, unsigned long s);

#endif /* RNG_YMER_H */
