/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: foxglynn.h,v $
*	$Revision: 1.1 $
*	$Date: 2007/10/01 12:39:50 $
*	$Author: zapreevis $
*	
*	MRMC is a model checker for discrete-time and continuous-time Markov reward models.
*	It supports reward extensions of PCTL and CSL (PRCTL and CSRL), and allows for the
*	automated verification of properties concerning long-run and instantaneous rewards
*	as well as cumulative rewards.
*	
*	Copyright (C) The University of Twente, 2004-2006.
*	Authors: Ivan Zapreev
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
*	Source description: Define FoxGlynn.
*	Reference: 
*		Computing Poisson Probabilities, Bennett L. Fox and Peter W. Glynn,
*		Communication of the ACM, Vol. 31, pp. 440-445 (1998).
*/

#include "macro.h"

#include <math.h>

#ifndef FOXGLYNN_H

#define FOXGLYNN_H

/*****************************************************************************
			STRUCTURE
name: FoxGlynn
@member left: The left truncation point
@member right: The right truncation point
@member total_weight: total weight based on input underflow and overflow
@member weights: array of double: weights - poisson probabilities*total_weight
remark: number of elements in weights = right - left.
******************************************************************************/
typedef struct FoxGlynn
{
	int left;
	int right;
	double total_weight;
	double *weights;
} FoxGlynn;

/*****************************************************************************
Name		: fox_glynn
Role		: get poisson probabilities.
@param		: double lambda: (rate of uniformization)*(mission time)
@param		: double tau: underflow
@param		: double omega: overflow
@param		: double epsilon: error bound
@param		: FoxGlynn **: return a new FoxGlynn structure by reference
@return	: TRUE if it worked fine, otherwise false
remark	    	:
******************************************************************************/
BOOL fox_glynn(const double lambda, const double tau, const double omega, const double epsilon, FoxGlynn **ppFG);

/**
* Fries the memory allocated for the FoxGlynn structure
* @param fg the structure to free
*/
void freeFG(FoxGlynn * fg);

#endif

