/**
*	WARNING: Do Not Remove This Section
*
*	$RCSfile: transient_ctmc.h,v $
*	$Revision: 1.1 $
*	$Date: 2007/10/01 12:39:56 $
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
*	Source description: Perform Transient Analysis for CSL - X, U.
*/

#include "macro.h"

#include <math.h>
#include <sys/types.h>

#include "foxglynn.h"
#include "bitset.h"
#include "sparse.h"
#include "runtime.h"
#include "lump.h"
#include "transient_common.h"
#include "bscc.h"
#include "iterative_solvers.h"

#ifndef TRANSIENT_CTMC_H

#define TRANSIENT_CTMC_H

/**
* Solve the unbounded until operator for all states.
* @param		: bitset *phi: SAT(phi).
* @param		: bitset *psi: SAT(psi).
* @return	: double *: result of the unbounded until operator for all states.
* NOTE: adapted from
* 	 1. F. Ciesinski, M. Grober. On Probabilistic Computation Tree Logic.
* 	 LNCS, Vol. 2925, Springer, pp. 147-188, 2004.
*/
double * unbounded_until(bitset *phi, bitset *psi);

/**
* Solve the unbounded until operator for all states with lumping.
* @param: bitset *phi: SAT(phi).
* @param: bitset *psi: SAT(psi).
* @return: double *: result of the unbounded until operator for all states.
*/
double * unbounded_until_lumping(bitset *phi, bitset *psi);

/**
* Solve the bounded until operator.
* @param		: bitset *phi: SAT(phi).
* @param		: bitset *psi: SAT(psi).
* @param		: double supi: sup I
* @return	: double *: result of the unbounded until operator for all states.
* NOTE: 1. J.-P. Katoen, M.Z. Kwiatowska, G. Norman, D.A. Parker.
* 	Faster and symbolic CTMC model checking. LNCS Vol. 2165, Springer,
* 	Aachen, Germany, pp. 23-38, 2001.
*/
double * bounded_until(bitset *phi, bitset *psi, double supi);

/**
* Solve the bounded until operator with lumping.
* @param: bitset *phi: SAT(phi).
* @param: bitset *psi: SAT(psi).
* @param: double supi: sup I
* @return: double *: result of the unbounded until operator for all states.
*/
double * bounded_until_lumping(bitset *phi, bitset *psi, double supi);

/**
* @param: bitset *phi: SAT(phi).
* @param: bitset *psi: SAT(psi).
* @param: double subi: sub I
* @param: double supi: sup I
* @return: double *: result of the interval until operator for all states.
* NOTE: 1. C.Baier, B.R. Haverkort, H. Hermanns and J.-P. Katoen.
* 	Model Checking Algorithms for Continuous-Time Markov Chains.
* 	IEEE Transactions on Software Engineering, Vol. 29, No. 6,
* 	pp. 299-316, 2000.
*/
double * interval_until(bitset *phi, bitset *psi, double subi, double supi);

/**
* Solve the interval until operator with lumping.
* @param: bitset *phi: SAT(phi).
* @param: bitset *psi: SAT(psi).
* @param: double subi: sub I
* @param: double supi: sup I
* @return: double *: result of the interval until operator for all states.
*/
double * interval_until_lumping(bitset *phi, bitset *psi, double subi, double supi);

/**
* Solve the unbounded next operator.
* @param: bitset *phi: SAT(phi).
* @return: double *: result of the unbounded next operator for states.
*/
double * unbounded_next(bitset *phi);

/**
* Solve the bounded next operator.
* @param: bitset *phi: SAT(phi).
* @param: double supi: sup I
* @return: double *: result of the bounded next operator for all states.
*/
double * bounded_next(bitset *phi, double supi);

/**
* Solve the interval next operator.
* @param: bitset *phi: SAT(phi).
* @param: double subi: sub I
* @param: double supi: sup I
* @return: double *: result of the interval next operator for all states.
*/
double * interval_next(bitset *phi, double subi, double supi);

#endif
