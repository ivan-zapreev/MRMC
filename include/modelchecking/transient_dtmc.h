/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: transient_dtmc.h,v $
*	$Revision: 1.1 $
*	$Date: 2007/10/01 12:39:57 $
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
*	Source description: Perform Transient Analysis for PCTL - X, U.
*/

#include "macro.h"

#include <math.h>
#include <sys/types.h>

#include "bitset.h"
#include "sparse.h"
#include "runtime.h"
#include "lump.h"
#include "transient_common.h"

#ifndef TRANSIENT_DTMC_H

#define TRANSIENT_DTMC_H

/**
* Solve the unbounded until operator for all states for DTMC.
* @param		: bitset *phi: SAT(phi).
* @param		: bitset *psi: SAT(psi).
* @return	: double *: result of unbounded until operator for all states.
* NOTE: 1. F. Ciesinski, M. Grober. On Probabilistic Computation Tree Logic.
* 	  LNCS, Vol. 2925, Springer, pp. 147-188, 2004.
*/
double * dtmc_unbounded_until(bitset *phi, bitset *psi);

/**
* Solve the unbounded until operator for all states for DTMC with lumping.
* @param		: bitset *phi: SAT(phi).
* @param		: bitset *psi: SAT(psi).
* @return		: double *: result of unbounded until operator for all states.
*/
double * dtmc_unbounded_until_lumping(bitset *phi, bitset *psi);

/**
* Solve the bounded until operator for DTMC.
* @param		: bitset *phi: SAT(phi).
* @param		: bitset *psi: SAT(psi).
* @param		: double supi: sup I should contain the Natural number
* @return	: double *: result of the unbounded until operator for all states.
* NOTE: 1. F. Ciesinski, M. Grober. On Probabilistic Computation Tree Logic.
* 	  LNCS, Vol. 2925, Springer, pp. 147-188, 2004.
*/
double * dtmc_bounded_until(bitset *phi, bitset *psi, double supi);

/**
* Solve the bounded until operator for DTMC with lumping.
* @param		: bitset *phi: SAT(phi).
* @param		: bitset *psi: SAT(psi).
* @param		: double supi: sup I should contain the Natural number
* @return		: double *: result of the unbounded until operator for all states.
*/
double * dtmc_bounded_until_lumping(bitset *phi, bitset *psi, double supi);

/**
* This method modelchecks the Xphi formula for the DTMC
* @param the phi states
* @return the probabilities
*/
double * dtmc_unbounded_next(bitset *phi);

#endif
