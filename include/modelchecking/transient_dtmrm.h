/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: transient_dtmrm.h,v $
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
*	Source description: Perform Transient Analysis for PRCTL - X, U.
*/

#include "macro.h"

#include <sys/types.h>

#include "runtime.h"
#include "bitset.h"
#include "sparse.h"
#include "path_graph.h"
#include "transient_common.h"
#include "lump.h"

#ifndef TRANSIENT_DTMRM_H

#define TRANSIENT_DTMRM_H

/**
* Solve the N-J-bounded until operator for DTMRM.
* Adds optimization for the case subi == 0 & subj == 0
* @param: bitset *phi: SAT(phi).
* @param: bitset *psi: SAT(psi).
* @param: double supi: sup I, I is a time interval
* @return: double *: result of the N-J-bounded until operator for all states.
* NOTE: see (slightly modified path_graph):
*	1. S. Andova, H. Hermanns and J.-P. Katoen.
*	Discrete-time rewards model-checked.
*	FORMATS 2003, LNCS, Vol. 2791, Springer, pp. 88 - 103, 2003.
*/
double * dtmrm_bounded_until(bitset *phi, bitset *psi, double subi, double supi, double subj, double supj);

/**
* Solve the N-J-bounded until operator for DTMRM with formula dependent lumping.
* Adds optimization for the case subi == 0 & subj == 0
* NOTE: NO support for impulse rewards!!!
* @param: bitset *phi: SAT(phi).
* @param: bitset *psi: SAT(psi).
* @param: double supi: sup I, I is a time interval
* @return: double *: result of the N-J-bounded until operator for all states.
* NOTE: see (slightly modified path_graph):
*	1. S. Andova, H. Hermanns and J.-P. Katoen.
*	Discrete-time rewards model-checked.
*	FORMATS 2003, LNCS, Vol. 2791, Springer, pp. 88 - 103, 2003.
*/
double * dtmrm_bounded_until_lumping(bitset *phi, bitset *psi, double subi, double supi, double subj, double supj);

#endif
